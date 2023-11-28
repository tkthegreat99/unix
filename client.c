#include "mytest.h"
#include "project.h"

void	do_compute_node()
{

}

// 8 * id - 8 + 1 <= value % 32 <= 8 * id;
void	do_comm_node(int id, int **client2client[], int pip[2])
{
	char	file_name[10];
	int		fd;
	int		data[MB];
	int		dump[MB];
	int		ret[4][2];
	int		i;
	int		j;

	sprintf(file_name, "p%d.dat", id);
	fd = open(file_name, O_CREAT | O_WRONLY | O_TRUNC, 0644);
	read(fd, data, MB);
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
			write(client2client[id][1], data[i], 4);
		else if (9 <= remain && remain <= 16)
			write(client2client[id][1], data[i], 4);
		else if (17 <= remain && remain <= 24)
			write(client2client[id][1], data[i], 4);
		else if (25 <= remain && remain <= 32)
			write(client2client[id][1], data[i], 4);
		else
		{
			perror("[Error] Exit (1)");
			exit(1);
		}
	}
	while (1) // 자기 자신에 대한 예외처리 주의
	{
		for (i = 0; i < 4; i++)
			ret[i][0] = read(client2client[i][id][0], ret[i][1], 4);
		for (i = 0; i < 4; i++)
		{
			if (ret[i][0] != 0)
				dump[++dump_idx] = ret[i][1];
		}
		if (ret[0][0] == 0 && ret[1][0] == 0 && ret[2][0] == 0 && ret[3][0] == 0)
			break ;
	}
	for (i = 0; i < 4; i++)
		close(client2client[id][i][1]); // id -> i :  close(write)
	for (i = 0; i < 4; i++)
		close(client2client[i][id][0]); // i -> id : close(read)

	// 정렬


	// 부모에게 넘기기
	



	close(pip[1]);
}

void	do_io_node(int id, int pip[2])
{
	close(pip[1]);

}

void	parent()
{
	int	pid;
	int	status;
	int	i;
	int	j;

	for (i = 0; i < 4; i++)
	{
		pid = waitpid(-1, &status, 0);
		if (pid == -1)
		{
			perror("pid error");
			exit(1);
		}
		printf("[DEBUG] pid : %d, status: %d done\n", pid, status);
	}
}

void	Client2Server(int i, int **client2client[])
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
		do_io_node(i, pip);
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
	parent();
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

