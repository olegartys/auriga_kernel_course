/*
 * test.c
 * 
 * Copyright 2016 olegartys <olegartys@olegartys-HP-Pavilion-15-Notebook-PC>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */


#include <stdio.h>

#include "allocator.h"

int main(int argc, char **argv)
{
	void *mem = malloc(sizeof(char)*100);
	free(mem);
	void *mem1 = calloc(1000, sizeof(char));
	memset(mem1, 0xFF, 1000);
	void *mem2 = realloc(mem, 2000);
	free(mem1);
	free(mem2);
	return 0;
}
