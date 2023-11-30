#include "mytest.h"
#include "project.h"

void	debug_result(void)
{
	int		i;
	char	cmd[1024];
	int		fd;
	int		buffer[MB];

	for (i = 1; i <= 4; i++)
	{
		sprintf(cmd, "od -i IOnode_#%d | more", i);
		system(cmd);
	}
	for (i = 1; i <= 4; i++)
	{
		sprintf(cmd, "IOnode_#%d", i);
		fd = open(cmd, O_RDONLY);
		printf("%d result byte: %d, MB byte: %d\n", i, (int)read(fd, buffer, MB * sizeof(int)), (int)(MB * sizeof(int)));
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
void	do_comm_node(int id, int client2client[4][4][2], int pip[2])
{
	char	file_name[10];
	int		fd;
	int		data[MB];
	int		dump[MB];
	int		ret;
	int		i;
	int		dump_idx;

	sprintf(file_name, "p%d.dat", id + 1);
	fd = open(file_name, O_RDONLY);
	if (fd < 0)
		exit(1);
	read(fd, data, MB * sizeof(int));
	close(pip[0]);
	for (i = 0; i < 4; i++)
		close(client2client[id][i][0]); // id -> i :  close(read)
	for (i = 0; i < 4; i++)
		close(client2client[i][id][1]); // i -> id : close(write)
	ret = 0;
	dump_idx = -1;
	for (i = 0; i < 4; i++)
	{
		if (i == id)
			continue;
		fcntl(client2client[i][id][0], F_SETFL, O_NONBLOCK);
	}
	for (i = 0; i < MB; i++)
	{
		int remain = data[i] % 32;

		if ((8 * id + 1 <= remain && remain <= 8 * id + 8) || (id == 3 && remain == 0))
			dump[++dump_idx] = data[i];
		else if (1 <= remain && remain <= 8)
			ret = write(client2client[id][0][1], &data[i], sizeof(int));
		else if (9 <= remain && remain <= 16)
			ret = write(client2client[id][1][1], &data[i], sizeof(int));
		else if (17 <= remain && remain <= 24)
			ret = write(client2client[id][2][1], &data[i], sizeof(int));
		else if ((25 <= remain && remain < 32) || remain == 0)
			ret = write(client2client[id][3][1], &data[i], sizeof(int));
		else
		{
			char	buffer[1024];

			sprintf(buffer, "[Error : %d] %d", id, remain % 512);
			perror(buffer);
			exit(1);
		}
		int	j;

		for (j = 0; j < 4; j++)
		{
			int	buffer;

			if (id == j)
				continue;
			// read쪽 pipe buffer 계속 비워줘야 한다.
			ret = read(client2client[j][id][0], &buffer, sizeof(int));
			if (ret > 0)
				dump[++dump_idx] = buffer;
		}
	}
	// NON-BLOCKING -> busy-waiting 발생
	// BLOCKING -> DEADLOCK 발생
	while (1) 
	{
		for (i = 0; i < 4; i++)
		{
			int	buffer;

			if (id == i)
				continue;
			ret = read(client2client[i][id][0], &buffer, sizeof(int));
			if (ret > 0)
				dump[++dump_idx] = buffer;
		}
		// dump_idx == MB - 1의 의미는 dump[MB - 1]에 write를 완료
		if (dump_idx == MB - 1)
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
	char	file_name[25];
	int		fd;

	close(pip[1]);
	sprintf(file_name, "IOnode_#%d", id + 1);
	printf("file_name : [%s]\n", file_name);
	fd = open(file_name, O_CREAT | O_WRONLY | O_TRUNC, 0644);
	if (fd < 0)
	{
		perror("I/O node CREATE FAIL");
		exit(1);
	}
	while ((ret = read(pip[0], chunk, sizeof(int) * 8)) > 0)
		write(fd, chunk, sizeof(int) * 8);
	close(pip[0]);
	close(fd);
}

void	parent(char *str)
{
	int		pid;
	int		status;
	int		i;
	char	buffer[1024];

	for (i = 0; i < 4; i++)
	{
		pid = waitpid(-1, &status, 0);
		if (pid == -1)
		{
			sprintf(buffer, "[PID ERROR : %s]", str);
			perror(buffer);
			exit(1);
		}
		printf("[DEBUG] %s, pid : %d, status: %d done\n", str, pid, status);
	}
}

void	Client2Server(int i, int client2client[4][4][2])
{
#ifdef TIMES
	t_mytime	io_time;
	t_mytime	comm_time;
	t_mytime	rest_time;
#endif
	int	pid;
	int	pip[2];
	int	status;

	pipe(pip);
	pid = fork();
	if (pid == 0) // client
		do_comm_node(i, client2client, pip);
	else // server
	{
		do_io_node(i, pip);
		wait(&status);
		printf("[DEBUG] Client2Server, pid : %d, status: %d done\n", pid, status);
	} 
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
		if (pid == 0)
		{
			Client2Server(i, client2client);
			exit(0);
		}
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
	return (1);
}

