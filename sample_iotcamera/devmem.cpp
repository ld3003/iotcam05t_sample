/*
 * devmem2.c: Simple program to read/write from/to any location in memory.
 *
 *  Copyright (C) 2000, Jan-Derk Bakker (J.D.Bakker@its.tudelft.nl)
 *
 *
 * This software has been developed for the LART computing board
 * (http://www.lart.tudelft.nl/). The development has been sponsored by
 * the Mobile MultiMedia Communications (http://www.mmc.tudelft.nl/)
 * and Ubiquitous Communications (http://www.ubicom.tudelft.nl/)
 * projects.
 *
 * The author can be reached at:
 *
 *  Jan-Derk Bakker
 *  Information and Communication Theory Group
 *  Faculty of Information Technology and Systems
 *  Delft University of Technology
 *  P.O. Box 5031
 *  2600 GA Delft
 *  The Netherlands
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "devmem.h"


#define FATAL                                                    \
	do                                                           \
	{                                                            \
		fprintf(stderr, "Error at line %d, file %s (%d) [%s]\n", \
				__LINE__, __FILE__, errno, strerror(errno));     \
		exit(1);                                                 \
	} while (0)

#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)

unsigned long devmem(off_t target, unsigned long writeval)
{
	int fd;
	void *map_base, *virt_addr;
	unsigned long read_result;

	int access_type = 'w';

	if ((fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1)
		FATAL;
	printf("/dev/mem opened.\n");
	fflush(stdout);

	/* Map one page */
	map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, target & ~MAP_MASK);
	if (map_base == (void *)-1)
		FATAL;
	printf("Memory mapped at address %p.\n", map_base);
	fflush(stdout);

	virt_addr = map_base + (target & MAP_MASK);
	switch (access_type)
	{
	case 'b':
		read_result = *((unsigned char *)virt_addr);
		break;
	case 'h':
		read_result = *((unsigned short *)virt_addr);
		break;
	case 'w':
		read_result = *((unsigned long *)virt_addr);
		break;
	default:
		fprintf(stderr, "Illegal data type '%c'.\n", access_type);
		exit(2);
	}
	printf("Value at address 0x%X (%p): 0x%X\n", target, virt_addr, read_result);
	fflush(stdout);

	switch (access_type)
	{
	case 'b':
		*((unsigned char *)virt_addr) = writeval;
		read_result = *((unsigned char *)virt_addr);
		break;
	case 'h':
		*((unsigned short *)virt_addr) = writeval;
		read_result = *((unsigned short *)virt_addr);
		break;
	case 'w':
		*((unsigned long *)virt_addr) = writeval;
		read_result = *((unsigned long *)virt_addr);
		break;
	}
	printf("Written 0x%X; readback 0x%X\n", writeval, read_result);
	fflush(stdout);

	if (munmap(map_base, MAP_SIZE) == -1)
		FATAL;
	close(fd);
	return read_result;
}