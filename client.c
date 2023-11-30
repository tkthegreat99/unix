#include "mytest.h"
#include "project.h"

void	debug_result(void)
{
	int	i;
	char	cmd[1024];

	i = 0;
	for (i = 1; i <= 4; i++)
	{
		sprintf(cmd, "I/O node #%d | more", i);
		system(cmd);
	}
}

// 오름차순 비교 함수 구현
int ft_compare(const void *a, const void *b)
{
    int num1 = *(int *)a;
    int num2 = *(int *)b;

    if (num1 < num2)
        return (-1);
    if (num1 > num2)
        return (1);
    return (0);
}

void	do_compute_node(int *dump)
{
	// 정렬
	qsort(dump, MB, sizeof(int), ft_compare);
}

// 8 * id - 8 + 1 <= value % 32 <= 8 * id;
void	do_comm_node(int id, int **client2client[], int pip[2])
{
	char	file_name[10];
	int		fd;
	int		data[MB];
	int		dump[MB];
	int		ret;
	int		count;
	int		i;

	sprintf(file_name, "p%d.dat", id);
	fd = open(file_name, O_CREAT | O_WRONLY | O_TRUNC, 0644);
	read(fd, data, MB * sizeof(int));
	close(pip[0]);
	for (i = 0; i < 4; i++)
		close(client2client[id][i][0]); // id -> i :  close(read)
	for (i = 0; i < 4; i++)
		close(client2client[i][id][1]); // i -> id : close(write)
	int	dump_idx = -1;
	for (i = 0; i < MB; i++)
	{
		int remain = data[i] % 32;
		if (8 * id - 8 + 1 <= remain && remain <= 8 * id)
			dump[++dump_idx] = data[i];
		else if (1 <= remain && remain <= 8)
			write(client2client[id][1][1], &data[i], sizeof(int));
		else if (9 <= remain && remain <= 16)
			write(client2client[id][2][1], &data[i], sizeof(int));
		else if (17 <= remain && remain <= 24)
			write(client2client[id][3][1], &data[i], sizeof(int));
		else if (25 <= remain && remain <= 32)
			write(client2client[id][4][1], &data[i], sizeof(int));
		else
		{
			perror("[Error] Exit (1)");
			exit(1);
		}
	}
	count = 0;
	while (1) // 자기 자신에 대한 예외처리 주의
	{
		for (i = 0; i < 4; i++)
		{
			int	buffer;

			ret = read(client2client[i][id][0], &buffer, sizeof(int));
			if (ret != 0)
				dump[++dump_idx] = buffer;
			else
				count++;
		}
		if (count == 4)
			break ;
	}
	for (i = 0; i < 4; i++)
		close(client2client[id][i][1]); // id -> i :  close(write)
	for (i = 0; i < 4; i++)
		close(client2client[i][id][0]); // i -> id : close(read)
	do_compute_node(dump); // 정렬하기
	// 부모에게 넘기기
	i = 0;
	while (i < MB)
	{
		write(pip[1], &dump[i], sizeof(int) * 8);
		i += 8;
	}
	close(pip[1]);
}

void	do_io_node(int id, int pip[2])
{
	int		ret;
	int		chunk[8];
	char	file_name[15];
	int		fd;

	close(pip[1]);
	sprintf(file_name, "I/O node #%d", id);
	fd = open(file_name, O_CREAT | O_WRONLY | O_TRUNC, 0644);
	while ((ret = read(pip[0], chunk, sizeof(int) * 8)) > 0)
		write(fd, chunk, sizeof(int) * 8);
	close(pip[0]);
	close(fd);
}

void	parent(char *str)
{
	int	pid;
	int	status;
	int	i;

	for (i = 0; i < 4; i++)
	{
		pid = waitpid(-1, &status, 0);
		if (pid == -1)
		{
			perror("pid error");
			exit(1);
		}
		printf("[DEBUG] %s, pid : %d, status: %d done\n", str, pid, status);
	}
}

void	Client2Server(int i, int ***client2client)
{
#ifdef TIMES
	t_mytime	io_time;
	t_mytime	comm_time;
	t_mytime	rest_time;
#endif
	int	pid;
	int	pip[2];

	pipe(pip);
	pid = fork();
	if (pid == 0) // server
	{
		do_io_node(i, pip);
		parent("Client2Server");
	}
	else // client
		do_comm_node(i, client2client, pip);
#ifdef TIMES
	t_mytime	io_time;
	t_mytime	comm_time;
	t_mytime	rest_time;
#endif
}

void	parallel_operation(void)
{
	int	client2client[4][4][2];
	int	i;
	int	j;

	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 4; j++)
			pipe(client2client[i][j]);
	}
	for (i = 0; i < 4; i++)
	{
		int	pid;

		pid = fork();
		if (pid < 0)
		{
			perror("fork error");
			exit(1);
		}
		else if (pid == 0)
			Client2Server(i, client2client);
	}
	parent("parallel_operation");
	debug_result();
}

int client_oriented_io() {

#ifdef TIMES
	struct timeval stime,etime;
	int time_result;

	t_mytime	io_time;
	t_mytime	comm_time;
	t_mytime	rest_time;
#endif
	/* Client_oriented_io. Measure io time, communication time, and time for the rest.
	*/

#ifdef TIMES
	gettimeofday(&stime, NULL);
#endif
	parallel_operation();

#ifdef TIMES
	gettimeofday(&etime, NULL);
	time_result = etime.tv_usec - stime.tv_usec;
	printf("Client_oriented_io TIMES == %ld %ld %ld\n", etime.tv_usec, stime.tv_usec, time_result);
#endif

}

