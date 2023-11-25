#include "mytest.h"
#include "project.h"

void	do_comm_node()
{
	


}

void	do_compute_node()
{
	t_mytime	comp_time;

	gettimeofday(&comp_time.stime, NULL);





	gettimeofday(&comp_time.etime, NULL);
	comp_time.time_result = comp_time.etime.tv_usec - comp_time.etime.tv_usec;
	printf("**Program compute node\n");
	printf("%ld %ld %ld\n", comp_time.etime.tv_usec, comp_time.stime.tv_usec, comp_time.time_result);
}

void	do_io_node()
{

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

// client와 server로 구성된 child를 4개 생성한다.
// (1) comm. client끼리 통신
// (2) comp. client 내부에서 정렬
// (3) I/O. server에서 write로 pipe로 전달받은 chunks를 저장한다.
// 의문점: 시간을 어떻게 측정할 것인가?
// (1) 임의의 child에서 위의 (1), (2), (3)만 시간을 측정하면 되는 것인가?
// (2) 병렬적인 시간 측정은 (1) 통신에 걸린 시간, (2) 정렬에 걸린 시간, (3) 정렬에 걸린 시간을 각각 종합해서 계산해야 하는 것 아닌가?
// 그렇다면 종합해서 시간 측정을 어떻게 하는가?
// 예를 들어, (1) 과정을 위해 client fork() 수행 및 시간 측정 시작 -> 통신 -> parent에서 자원 회수 및 시간 측정 종료?
// 이렇게 해야 하는건가? 그렇다면 각각의 process가 점유한 자원을 어떻게 관리하는가?
void	parallel_operation(t_mytime *io_time, t_mytime *comm_time)
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
			child(i, client2client, io_time, comm_time);
	}
	parent();
}

int client_oriented_io() {

#ifdef TIMES
	struct timeval stime,etime;
	int time_result;

	t_mytime	io_time;
	t_mytime	comm_time;
#endif
	/* Client_oriented_io. Measure io time, communication time, and time for the rest.
	*/

#ifdef TIMES
	gettimeofday(&stime, NULL);
#endif

#ifdef	TIMES
	parallel_operation(&io_time, &comm_time);
#endif
	printf("**Program IO, communication and the rest\n");
	printf("**Program IO\n");
#ifdef	TIMES
	io_time.time_result = io_time.etime.tv_usec - io_time.stime.v_usec;
	printf("%ld %ld %ld\n", io_time.etime.tv_usec, io_time.stime.tv_usec, comm_time.time_result);
#endif
	printf("**Program communication\n");
#ifdef	TIMES
	comm_time.time_result = comm_time.etime.tv_usec - comm_time.stime.tv_usec;
	printf("%ld %ld %ld\n", comm_time.etime.tv_usec, comm_time.stime.tv_usec, comm_time.time_result);
#endif

#ifdef TIMES
	gettimeofday(&etime, NULL);
	time_result = etime.tv_usec - stime.tv_usec;
	printf("Client_oriented_io TIMES == %ld %ld %ld\n", etime.tv_usec, stime.tv_usec, time_result);
#endif

}

