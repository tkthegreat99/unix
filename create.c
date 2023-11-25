#include "project.h"
#include "mytest.h"

void    child_proc(int id)
{
	char	file_name[5];
	char	cmd[1024];
	int	fd;
	int	data[MB + 1];

	sprintf(file_name, "p%d.dat", id);
	fd = open(file_name, O_CREAT | O_WRONLY | O_TRUNC, 0644);
	if (fd == -1)
	{
		perror("fd fail");
		exit(1);
	}
	for (int i = 0; i <= MB; i++)
		data[i] = 4 * i + id;
	if (write(fd, data, (MB + 1) * 4) < 0)
	{
		perror("write fail");
		exit(1);
	}
	sprintf(cmd, "od -i p%d.dat | more", id);
	system(cmd);
}

int create_source_data() {

	/* create per-process, distrinuted input data. The size of each proc is of 1MB (256K of integer).
	create one time, if possible. After creating, comment out to program the remaining functions. */

	printf("**Distribute input data across processes.\n");
	
	int     i;
	int     pid;

	i = 0;
	for (i = 1; i <= 4; i++)
	{
		pid = fork();
		if (pid == -1)
		{
			perror("fork fail");
			exit(1);
		}
		else if (pid == 0)
			child_proc(i);
	}
	i = 0;
	int     status;
	while (i < 4)
	{
		pid = waitpid(-1, &status, 0);
		if (pid == -1)
		{
			perror("pid error");
			exit(1);
		}
		printf("[DEBUG] pid : %d, status: %d done\n", pid, status);
		i++;
	}
	return (0);
}
