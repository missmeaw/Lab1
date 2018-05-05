#include "unistd.h"
#include "stdio.h"
#include "dirent.h"
#include "fcntl.h"
#include "sys/types.h"
#include "string.h"
#include "sys/stat.h"
#include "stdlib.h"
#include "errno.h"

#define BS 1024
#define BS3 256

void Pack(char *dir)
{
	DIR *d = NULL;
	struct dirent *sd;
	struct stat ss;
	int f, in, nread;
	char buff[BS];
	size_t l;
	off_t l1;

	d = opendir(dir);
	if (d == NULL) {
		write(1, "Error opening Dir\n", 18);
		exit(11);
	}
	f = open("zzz", O_WRONLY | O_CREAT, 0775);
	chdir(dir);
	if (f < 0) {
		write(1, "Error opening file zzz\n", 23);
		exit(12);
	}
	while ((sd = readdir(d)) != NULL) {
		if (!(lstat(sd->d_name, &ss) == 0)) {
			write(1, "Error\n", 6);
			exit(13);
		}
		if (S_ISDIR(ss.st_mode)) {
			if (strcmp(".", sd->d_name) == 0 ||
				strcmp("..", sd->d_name) == 0)
				continue;
			l = strlen(sd->d_name);
			if (write(f, "/", 1) == -1) {
				write(1, "Error writing 1\n", 16);
				exit(14);
			}
			if (write(f, &l, sizeof(l)) == -1) {
				write(1, "Error writing 2\n", 16);
				exit(14);
			}
			if (write(f, &(sd->d_name), l) == -1) {
				write(1, "Error writing 3\n", 16);
				exit(14);
			}
		} else
			if ((strcmp("zzz", sd->d_name)) != 0) {
				l = strlen(sd->d_name);
				if (write(f, "*", 1) == -1) {
					write(1, "Error writing\n", 14);
					exit(14);
				}
				if (write(f, &l, sizeof(l)) == -1) {
					write(1, "Error writing\n", 14);
					exit(14);
				}
				if (write(f, sd->d_name, l) == -1) {
					write(1, "Error writing\n", 14);
					exit(14);
				}
				if (write(f, &ss.st_size,
					sizeof(ss.st_size)) == -1) {
					write(1, "Error writing\n", 14);
					exit(14);
				}
				in = open(sd->d_name, O_RDONLY);
				if (in < 0) {
					write(1, "Error open in\n", 14);
					exit(14);
				}
				l1 = ss.st_size;
				while (l1 > BS) {
					nread = read(in, buff, BS);
					if (nread == -1) {
						write(1,
						"Error reading1\n", 15);
						exit(14);
					}
					l1 = l1-BS;
					if (write(f, buff, BS) == -1) {
						write(1,
						"Error writing\n", 14);
						exit(14);
					}
				}
				if (read(in, buff, l1) == -1) {
					write(1,
					"Error reading\n", 14);
					exit(14);
				}
				if (write(f, buff, l1) == -1) {
					write(1,
					"Error writing\n", 14);
					exit(14);
				}
			}
	}
	write(1, "Complete\n", 9);
	if (closedir(d) == 0)
		exit(15);
	if (close(f) == 0)
		exit(15);
}

void clearbuf(char *buf, size_t l)
{
	for (int i = 0; i < l; i++)
		buf[i] = '\0';
}

void Unpack(char *dir)
{
	char *buf;
	char buf3[BS3] = {0}, b1, buff[BS];
	int f, out;
	int nread, j = 1;
	size_t l;
	off_t l1;
	int b = 0;


	f = open("zzz", O_RDONLY);
	if (f < 0) {
		write(1, "Error opening file zzz\n", 23);
		exit(12);
	}
	if (chdir(dir) == -1) {
		if (mkdir(dir, 0775) == -1) {
			write(1, "Error new dir\n", 14);
			exit(11);
		}
		chdir(dir);
	}
	while (read(f, &b1, 1)) {
		switch (b1) {
		case '/': {
			if (read(f, &l, sizeof(l)) == -1) {
				write(1, "Error reading l\n", 16);
				exit(13);
			}
			if (read(f, buf3, l) == -1) {
				write(1, "Error reading l1\n", 17);
				exit(13);
			}
			mkdir(buf3, 0775);
			clearbuf(buf3, l);
			break;
		}
		case '*': {
			if (read(f, &l, sizeof(l)) == -1) {
				write(1, "Error reading l\n", 16);
				exit(14);
			}
			if (read(f, buf3, l) == -1) {
				write(1, "Error reading name\n", 19);
				exit(14);
			}
			out = open(buf3, O_WRONLY | O_CREAT, 0755);
			if (out < 0) {
				write(1, "Error create file\n", 18);
				exit(14);
			}
			clearbuf(buf3, l);
			if (read(f, &l1, sizeof(l1)) == -1) {
				write(1, "Error reading Nbytes\n", 20);
				exit(14);
			}
			while (l1 > BS) {
				if (read(f, buff, BS) == -1) {
					write(1,
					"Error read file\n", 16);
					exit(14);
				}
				l1 = l1-BS;
				if (write(out, buff, BS) == -1) {
					write(1, &errno, sizeof(errno));
					write(1,
					"Error writing buf\n", 18);
					exit(14);
				}
			}
			if (read(f, buff, l1) == -1) {
				write(1, "Error read file\n", 16);
				exit(14);
			}
			if (write(out, buff, l1) == -1) {
				write(1, "Error writing\n", 14);
				exit(14);
			}
			break;
		}
		case '\0': {
			write(1, "No zip file\n", 12);
			exit(15);
		}
		default: {
			printf("%c", b1);
			write(1, "Error reading b1\n", 17);
			exit(15);
			break;
		}
		}
	}
}

void main(void)
{
	char buf[BS3];
	char i, j;
	int n;

	write(1, "Press the key:\n 1 - Packing\n", 28);
	write(1, " 2 - Unpacking\n Any key - Exit\n", 31);
	if (read(0, &i, 1) < 1) {
		write(1, "Error reading key\n", 18);
		exit(1);
	}
	if (read(0, &j, 1) < 1) {
		write(1, "Error reading\n", 14);
		exit(1);
	}
	switch (i) {
	case '1':
	{
		write(1, "Write dirent\n", 13);
		n = read(0, buf, sizeof(buf));
		if (n < 2) {
			write(1, "Error reading dirent\n", 21);
			exit(1);
		}
		buf[n-1] = '\0';
		Pack(buf);
		clearbuf(buf, n);
		break;
	}
	case '2':
	{
		write(1, "Write dirent\n", 13);
		n = read(0, &buf, sizeof(buf));
		if (n < 2) {
			write(1, "Error reading dirent\n", 21);
			exit(1);
		}
		buf[n-1] = '\0';
		Unpack(buf);
		clearbuf(buf, n);
		break;
	}
	default:
	{
		write(1, "Good bye!\n", 10);
		i = '0';
		break;
	}
	}
}
