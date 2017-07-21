#include "disk.h"

#include "error.h"
#include "mem.h"
#include "port.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

/////////////////////////////////////////////////////////////////////////
// Internal constants + helper macros
/////////////////////////////////////////////////////////////////////////

#define IS_VALID_DISK(disk) ((disk) < DISK_MAX_DISKS)
#define IS_VALID_SIZE(size) (MEM_BLOCK_MASK(size) == 0)

#define BEGIN_MMAP_ADDR (MEM_BLK_SIZE * (MEM_NUM_BLKS - DISK_MAX_DISKS))
#define DISK_MMAP_ADDR(disk) (BEGIN_MMAP_ADDR + (disk * MEM_BLK_SIZE))

/////////////////////////////////////////////////////////////////////////
// Module internal declarations
/////////////////////////////////////////////////////////////////////////

typedef struct _disk_info_entry {
	const char *name;
	FILE *file;
	size_t fsize;

	disk_block *buffer;
	bool active;

	port_id cmd_port;
	port_id data_port;

	disk_addr off; // The offset of the window into the file
} disk_info_entry;

// Every entry is initialized to empty
static disk_info_entry disks[DISK_MAX_DISKS];

static error_t sync_disk(disk_id num);

static error_t seek_disk(disk_id num, disk_addr new_off, bool sync_first);

static error_t bind_disk(disk_id num, const char *filename);

static error_t unbind_disk(disk_id num, error_t partial);

/**
 * Returns the lowest-numbered disk unused (ready to be allocated).
 */
static disk_id next_unused();

/**
 * Marks a disk as available for reuse.
 */
static void mark_unused(disk_id num);

typedef struct _disk_operation {
	disk_action act;
	disk_state res;
	uint32_t data;
} disk_operation;

// Every disk has a command state with it
static disk_operation curr_op[DISK_MAX_DISKS];

static disk_id identify_disk(port_id port);

static void command_recv(port_id num, uint32_t command);
static uint32_t command_reply(port_id num);

static void data_write(port_id num, uint32_t data);
static uint32_t data_read(port_id num);

static void write_operation(disk_info_entry *disk, disk_operation *action);
static uint32_t read_operation(disk_info_entry *disk, disk_operation *action);

// Every disk has the same port structure
static port_entry disk_port[] = {
	{"Disk v1 command", command_recv, command_reply},
	{"Disk v1 data", data_write, data_read}
};

/////////////////////////////////////////////////////////////////////////
// Interface functions
/////////////////////////////////////////////////////////////////////////

error_t disk_install(const char *filename, disk_id *num)
{
	*num = next_unused();

	error_t stat = bind_disk(*num, filename);

	if (stat == ERR_INVAL || stat == ERR_PCOND) {
		return ERR_PCOND;
	}

	// If we failed halfway through, we need to clean up
	if (stat != ERR_NOERR) {
		unbind_disk(*num, stat);
		mark_unused(*num);
	}

	return stat;
}

error_t disk_remove(disk_id num)
{
	error_t stat = unbind_disk(num, ERR_NOERR);

	// If the unbinding failed, the disk may not be able to be reused
	// So we don't attempt to reuse it
	if (stat == ERR_NOERR || stat == ERR_FILE) {
		mark_unused(num);
	}

	return stat;
}

/////////////////////////////////////////////////////////////////////////
// Module internal functions
/////////////////////////////////////////////////////////////////////////

error_t sync_disk(disk_id num)
{
	if (!IS_VALID_DISK(num)) {
		return ERR_INVAL;
	}

	disk_info_entry *curr = &disks[num];

	if (!curr->active) {
        return ERR_PCOND;
	}

	if (fseek(curr->file, curr->off, SEEK_SET) != 0) {
		return ERR_FILE;
	}

	fwrite(curr->buffer, 1, MEM_BLK_SIZE, curr->file);

	return ERR_NOERR;
}

error_t seek_disk(disk_id num, disk_addr new_off, bool sync_first)
{
	if (!IS_VALID_DISK(num)) {
		return ERR_INVAL;
	}

	disk_info_entry *curr = &disks[num];

	if ((curr->fsize - new_off) < MEM_BLK_SIZE) {
		// We still need space for a full block
		return ERR_INVAL;
	}

	if (!curr->active) {
        return ERR_PCOND;
	}

	if (sync_first) {
		if (sync_disk(num) != ERR_NOERR) {
			return ERR_FILE;
		}
	}

	if (fseek(curr->file, new_off, SEEK_SET) != 0) {
		return ERR_FILE;
	}
	curr->off = new_off;

	fread(curr->buffer, 1, MEM_BLK_SIZE, curr->file);
	return ERR_NOERR;
}

error_t bind_disk(disk_id num, const char *filename)
{
	if (!IS_VALID_DISK(num)) {
		return ERR_INVAL;
	}

	disk_info_entry *curr = &disks[num];

	if (curr->active) {
        return ERR_PCOND;
	}

	curr->name = filename;
	curr->active = true;

	curr->file = fopen(filename, "r+b");
	if (curr->file == NULL) {
        return ERR_FILE;
	}

	if (fseek(curr->file, 0, SEEK_END) != 0) {
		return ERR_EXTERN;
	}

	curr->fsize = (size_t)ftell(curr->file);
	if (curr->fsize < MEM_BLK_SIZE) {
		return ERR_EXTERN;
	}

	curr->buffer = malloc(MEM_BLK_SIZE);
	if (curr->buffer == NULL) {
		return ERR_NOMEM;
	}

	seek_disk(num, 0, false);

	mem_map_device(DISK_MMAP_ADDR(num), curr->buffer);

	error_t stat = port_install(&disk_port[0], &curr->cmd_port);
	if (stat != ERR_NOERR) {
		return ERR_PORT;
	}

	stat = port_install(&disk_port[1], &curr->data_port);
	if (stat != ERR_NOERR) {
		// Clean up the successfully created port first
		port_remove(curr->cmd_port);
		return ERR_PORT;
	}

	return ERR_NOERR;
}

error_t unbind_disk(disk_id num, error_t partial)
{
	if (!IS_VALID_DISK(num)) {
		return ERR_INVAL;
	}

	disk_info_entry *curr = &disks[num];

	if (!curr->active) {
        return ERR_PCOND;
	}

	error_t stat = ERR_NOERR;

	if (partial == ERR_NOERR) {
        // If the file was opened correctly (and potentially used)
        // Then we need to write out its contents
        stat = sync_disk(num);
	}

	curr->name = NULL;
	curr->active = false;
	curr->off = 0;
	curr->fsize = 0;

	if (partial == ERR_FILE) {
		return stat;
	}

	fclose(curr->file);
	curr->file = NULL;

	if (partial == ERR_NOMEM || partial == ERR_EXTERN) {
		return stat;
	}

	mem_unmap_device(DISK_MMAP_ADDR(num));

	free(curr->buffer);
    curr->buffer = NULL;

    if (partial == ERR_PORT) {
		return stat;
    }

	port_remove(curr->cmd_port);
	curr->cmd_port = 0;
	port_remove(curr->data_port);
	curr->data_port = 0;

	return stat;
}

// Used to hold state for the following two functions
static disk_id next_alloc;

disk_id next_unused()
{
	// First check the next_alloc variable
	// If it's good, we're good
	// Otherwise we have to go hunting
	if (disks[next_alloc].active) {
		for (disk_id i = 0; IS_VALID_DISK(i); ++i) {
			if (!disks[i].active) {
				next_alloc = i;
				break;
			}
		}
	}

	disk_id to_ret = next_alloc;

	if (!IS_VALID_DISK(++next_alloc)) {
		next_alloc = 0;
	}

	return to_ret;
}

void mark_unused(disk_id num)
{
	// We want to prefer low-numbered ports
	if (num < next_alloc) {
		next_alloc = num;
	}
}

disk_id identify_disk(port_id port)
{
	for (disk_id i = 0; IS_VALID_DISK(i); ++i) {
		if (disks[i].cmd_port == port || disks[i].data_port == port) {
			return i;
		}
	}

	return DISK_MAX_DISKS; // Guaranteed to be invalid
}

void command_recv(port_id num, uint32_t command)
{
	disk_id curr = identify_disk(num);

	// Only act if we're in a correct location
	if (curr != DISK_MAX_DISKS) {
		curr_op[curr].act = (int)command;

		if (curr_op[curr].act == DA_NONE) {
			curr_op[curr].res = DS_OK;
		}
		else {
			curr_op[curr].res = DS_WAIT;
		}
	}
}

uint32_t command_reply(port_id num)
{
	disk_id curr = identify_disk(num);

	// Automatic error if we can't identify which disk
	if (curr == DISK_MAX_DISKS) {
		return (uint32_t)DS_ERROR;
	}

	return (uint32_t)curr_op[curr].res;
}

void data_write(port_id num, uint32_t data)
{
	disk_id curr = identify_disk(num);

	// Automatic error if we can't identify which disk
	if (curr == DISK_MAX_DISKS) {
		return;
	}

	curr_op[curr].data = data;
	write_operation(&disks[curr], &curr_op[curr]);
}

uint32_t data_read(port_id num)
{
	disk_id curr = identify_disk(num);

	// Automatic error if we can't identify which disk
	if (curr == DISK_MAX_DISKS) {
		return 0;
	}

	// This needs special-casing because read_operation
	// Doesn't know the disk number
	if (curr_op[curr].act == DA_NUM) {
		curr_op[curr].res = DS_OK;
		return curr;
	}

	return read_operation(&disks[curr], &curr_op[curr]);
}

void write_operation(disk_info_entry *disk, disk_operation *action)
{
	if (!disk->active) {
		action->res = DS_ERROR;
		return;
	}

	switch (action->act) {
		default:
			action->res = DS_ERROR;
			return;
	}
}

uint32_t read_operation(disk_info_entry *disk, disk_operation *action)
{
	if (!disk->active) {
		action->res = DS_ERROR;
		return 0;
	}

	switch (action->act) {
		default:
			action->res = DS_ERROR;
			return 0;

		case DA_SEEK:
			action->res = DS_OK;
			return disk->off;
	}
}
