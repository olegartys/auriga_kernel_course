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
	/*void *mem = malloc(sizeof(char)*100);
	free(mem);
	void *mem1 = calloc(1000, sizeof(char));
	memset(mem1, 0xFF, 1000);
	void *mem2 = realloc(mem, 2000);
	free(mem1);
	free(mem2);*/
	int i;
	int *p1 = malloc(10*sizeof(int));
	for (i = 0; i < 10; i++) p1[i] = i;
	for (i = 0; i < 10; i++) if (p1[i] != i) puts("err1");	
	
	int *p2 = malloc(10*sizeof(int));
	for (i = 0; i < 10; ++i) p2[i] = -1;
	for (i = 0; i < 10; i++) if (p1[i] != i) puts("err2");
	for (i = 0; i < 10; i++) if (p2[i] != -1) puts("err3");	

	p1 = realloc(p1, 20 * sizeof(int));
	for (i = 0; i < 20; ++i) p1[i] = i;
	for (i = 0; i < 20; ++i) if (p1[i] != i) puts("err4");
	for (i = 0; i < 10; i++) {
		printf("p2[%d] = %d\n", i, p2[i]);
		 if (p2[i] != -1) puts("err5");	
	}

	p1 = realloc(p1, 5 * sizeof(int));
	for (i = 0; i < 5; i++) if (p1[i] != i) puts("err6");
	for (i = 0; i < 10; i++) if (p2[i] != -1) puts("err7");

	free(p2);
	free(p1);
 	
	return 0;
}

