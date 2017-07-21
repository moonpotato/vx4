#include "disk.h"

#include "error.h"
#include "mem.h"
#include "port.h"

#include <stdbool.h>
#include <stdint.h>

/////////////////////////////////////////////////////////////////////////
// Internal constants + helper macros
/////////////////////////////////////////////////////////////////////////

#define IS_VALID_DISK(disk) ((disk) < DISK_MAX_DISKS)
#define IS_VALID_SIZE(size) (MEM_BLOCK_MASK(size) == 0)

/////////////////////////////////////////////////////////////////////////
// Module internal declarations
/////////////////////////////////////////////////////////////////////////

typedef struct _disk_info_entry {
	const char *name;
	bool active;

	mem_addr loc; // Must be a multiple of MEM_BLK_SIZE
	disk_block *buffer;
	disk_addr off; // The offset of the window into the file

	port_id cmd_port;
	port_id data_port;
} disk_info_entry;

// Every entry is initialized to empty
static disk_info_entry disks[DISK_MAX_DISKS];

static error_t bind_disk(disk_id num, const char *filename);

static error_t unbind_disk(disk_id num);

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

	return bind_disk(*num, filename);
}

error_t disk_remove(disk_id num)
{
	error_t stat = unbind_disk(num);

	// If the unbinding failed, the disk may not be able to be reused
	// So we don't attempt to reuse it
	if (stat == ERR_NOERR) {
		mark_unused(num);
	}

	return stat;
}

/////////////////////////////////////////////////////////////////////////
// Module internal functions
/////////////////////////////////////////////////////////////////////////

static error_t bind_disk(disk_id num, const char *filename)
{

}

static error_t unbind_disk(disk_id num)
{

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

static uint32_t data_read(port_id num)
{
	disk_id curr = identify_disk(num);

	// Automatic error if we can't identify which disk
	if (curr == DISK_MAX_DISKS) {
		return 0;
	}

	return read_operation(&disks[curr], &curr_op[curr]);
}

static void write_operation(disk_info_entry *disk, disk_operation *action)
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

static uint32_t read_operation(disk_info_entry *disk, disk_operation *action)
{
	if (!disk->active) {
		action->res = DS_ERROR;
		return 0;
	}

	switch (action->act) {
		default:
			action->res = DS_ERROR;
			return 0;

		case DA_ADDR:
			action->res = DS_OK;
			return disk->loc;

		case DA_SEEK:
			action->res = DS_OK;
			return disk->off;
	}
}
