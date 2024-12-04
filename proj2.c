#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <semaphore.h>


typedef struct
{
	id_t IDAtom;
	char symbol;
} AtomData;


typedef struct
{
	pid_t IDAtom;
	AtomData Elem;
	unsigned int extra;
} ProcessData;


typedef struct
{	
	unsigned int Count_H;
	unsigned int Count_O;
	unsigned int NH, NO, TI, TB;
} ArgumentsData;


typedef struct
{
	id_t IDMolecule;
	key_t shmid;
	unsigned int molecule_counter_h;
	unsigned int line_n;
	unsigned int hydrogen_n;
	unsigned int oxygen_n;
} IPCProcess;


typedef struct
{	
	sem_t *sem_ready;
	sem_t*sem_created;
	sem_t*sem_creating;
	sem_t*sem__stop;
	sem_t *semaphore_H;
	sem_t *semaphore_O;
	sem_t*semaphore_out;
} Semaphores;



void ErrorsFunc(int);
ProcessData createProcess(unsigned int, const char);
int ArgumentsFunc(ArgumentsData *, int, char **);
ProcessData createProcess(unsigned int, const char);
void GlobalStatus(const int, ProcessData *, IPCProcess *);
void Next(Semaphores*);
void Prev(Semaphores*);
IPCProcess* ipcStart();
void FreeMemoryFromQueue(IPCProcess*);
int ArgumentsFunc(ArgumentsData *, int, char **);
#define init(name,value)(sem_open(name, O_CREAT | O_WRONLY, 0666, value))
#define ATOM_CREATE(IDAtom, symbol) ((AtomData){IDAtom, symbol})
#define PROCESS_CREATE(IDAtom, Elem) ((ProcessData){IDAtom, Elem, 0})



ProcessData createProcess(unsigned int n, const char symbol)
{
	for (id_t IDAtom = 1; IDAtom <= n; IDAtom++)
		if (fork() == 0)
			return PROCESS_CREATE(getpid(), ATOM_CREATE(IDAtom, symbol));

	return (ProcessData) { 0 };
}


void GlobalStatus(const int IdStatus, ProcessData *proc, IPCProcess *data)
{
	FILE *outfile = fopen("./proj2.out", "a");
	++data->line_n;	

	if (IdStatus == 20)
		fprintf(outfile,"%u: %c %u: started\n", data->line_n, proc->Elem.symbol, proc->Elem.IDAtom);
	else if (IdStatus == 21)
		fprintf(outfile,"%u: %c %u: going to queue\n", data->line_n, proc->Elem.symbol, proc->Elem.IDAtom);
	else if (IdStatus == 22)
		fprintf(outfile,"%u: %c %u: creating molecule %u\n", data->line_n, proc->Elem.symbol, proc->Elem.IDAtom, data->IDMolecule);
	else if (IdStatus == 23)
		fprintf(outfile,"%u: %c %u: molecule %u created\n", data->line_n, proc->Elem.symbol, proc->Elem.IDAtom, data->IDMolecule);
	else if (IdStatus == 24)
		fprintf(outfile,"%u: %c %u: not enough O or H\n", data->line_n, proc->Elem.symbol, proc->Elem.IDAtom);
	else
		fprintf(outfile,"%u: %c %u: not enough H\n", data->line_n, proc->Elem.symbol, proc->Elem.IDAtom);
	
	fclose(outfile);
}


IPCProcess* ipcStart() 
{
	key_t key = ftok("proj2.c", 'c');
	int ipc_key = shmget(key, sizeof(IPCProcess), 0666 | IPC_CREAT);
	IPCProcess* ipc = shmat(ipc_key, NULL, 0);
	ipc->shmid = ipc_key;

	return ipc;
}

void FreeMemoryFromQueue(IPCProcess*ipc) {

	key_t key = ipc->shmid;

	shmctl(key, IPC_RMID, NULL);

	shmdt(ipc);
}


void Next(Semaphores* sem)
{
	int f[7];
	f[0] = (sem->semaphore_H = init("semaphore_hydrogen", 0)) == SEM_FAILED;
	f[1] = (sem->semaphore_O = init("semaphore_oxygen", 1)) == SEM_FAILED;
	f[2] = (sem->semaphore_out = init("semaphore_output", 1)) == SEM_FAILED;
	f[3] = (sem->sem_ready = init("semaphore_ready", 0)) == SEM_FAILED;
	f[4] = (sem->sem_created = init("semaphore_create", 0)) == SEM_FAILED;
	f[5] = (sem->sem_creating = init("semaphore_creating", 0)) == SEM_FAILED;
	f[6] = (sem->sem__stop = init("semaphore_stop_extra", 0)) == SEM_FAILED;

	for (int i = 0; i < 7; i++)
		if (f[i] == 1) {
			perror("sem_open");
			exit(1);
		}

	sem_unlink("semaphore_hydrogen");
	sem_unlink("semaphore_oxygen");
	sem_unlink("semaphore_output");
	sem_unlink("semaphore_ready");
	sem_unlink("semaphore_create");
	sem_unlink("semaphore_creating");
	sem_unlink("semaphore_stop_extra");
}


void Prev(Semaphores*sem) {
	sem_close(sem->semaphore_H);
	sem_close(sem->semaphore_O);
	sem_close(sem->semaphore_out);
	sem_close(sem->sem_ready);
	sem_close(sem->sem_created);
	sem_close(sem->sem_creating);
	sem_close(sem->sem__stop);
}

void ErrorsFunc(int error_code) {
	if (error_code == 0)
		return;

	if (error_code == 0)
		fprintf(stderr, "ERROR: Invalid input arguments\n");
	else
		fprintf(stderr, "ERROR: Unknown error IDAtom\n");		

	exit(1);
}


int ArgumentsFunc(ArgumentsData *arguments, int args_count, char **argumens_var)
{
	if (args_count != 5)
		return 10;

	for (int line = 1; line < args_count; line++)
	{
		if (argumens_var[line][0] == '\0')
			return 10;

		for (int argNum = 0; argumens_var[line][argNum]; argNum++)
			if (!(((argumens_var[line][argNum])>='0') && (argumens_var[line][argNum]) <= '9'))
				return 10;

		unsigned int intArgument = (unsigned)atoi(argumens_var[line]);

		if (line == 1)
		{
			if (intArgument < 1) 	return 1;
			arguments->NO = intArgument;
		}
		else if (line == 2)
		{
			if (intArgument < 1)	return 1;
			arguments->NH = intArgument;
		}
		else if (line == 3)
		{
			if (intArgument > 1000) return 1;
			arguments->TI = intArgument;
		}
		else
		{
			if (intArgument > 1000) return 1;
			arguments->TB = intArgument;
			break;
		}
	}
	arguments->Count_H = arguments->NH;
	arguments->Count_O = arguments->NO;
	if (arguments->NH % 2 == 1)
		arguments->Count_H--;
	if (arguments->NH >= arguments->NO * 2)
		arguments->Count_H = arguments->Count_O * 2;
	else
		arguments->Count_O = arguments->Count_H / 2;

	return 0;
}


int main(int argc, char **argv)
{	
	remove("./proj2.out");
	ArgumentsData arguments;
	ProcessData proc;
	Semaphores semaphor;
	IPCProcess *ipcproc = NULL;

	pid_t main_process_id = getpid();
	ErrorsFunc(ArgumentsFunc(&arguments, argc, argv));
	ipcproc = ipcStart();
	Next(&semaphor);

	if (arguments.Count_H == 0 && arguments.Count_O == 0)
		sem_post(semaphor.sem__stop);	
	if (main_process_id == getpid())	
		proc = createProcess(arguments.NO, 'O');	
	if (main_process_id == getpid())
		proc = createProcess(arguments.NH, 'H');

	if (main_process_id != getpid())
	{		
		srand(getpid());		
		sem_wait(semaphor.semaphore_out);
		GlobalStatus(20, &proc, ipcproc);
		sem_post(semaphor.semaphore_out);		
		usleep(arguments.TI > 0 ? rand() % (arguments.TI) * 1000 : 0);		
		sem_wait(semaphor.semaphore_out);
		GlobalStatus(21, &proc, ipcproc);
		sem_post(semaphor.semaphore_out);		
		switch (proc.Elem.symbol)
		{
		case 'H':
			ipcproc->hydrogen_n++;
			proc.extra = arguments.Count_H < ipcproc->hydrogen_n;
			break;
		case 'O':
			ipcproc->oxygen_n++;
			proc.extra = arguments.Count_O < ipcproc->oxygen_n;
			break;
		}
		
		if (proc.extra == 1)
		{
			sem_wait(semaphor.sem__stop);
			GlobalStatus(proc.Elem.symbol == 'H' ? 24 : 25, &proc, ipcproc);
			sem_post(semaphor.sem__stop);
			exit(0);
		}

		if (proc.Elem.symbol == 'H')
		{
			sem_wait(semaphor.semaphore_H);			
			sem_wait(semaphor.semaphore_out);
			GlobalStatus(22, &proc, ipcproc);
			sem_post(semaphor.semaphore_out);
			if (++ipcproc->molecule_counter_h >= 2)
			{
				sem_post(semaphor.sem_creating);
				sem_post(semaphor.sem_creating);
				sem_post(semaphor.sem_creating);
			}
			sem_wait(semaphor.sem_creating);
			sem_wait(semaphor.semaphore_out);
			GlobalStatus(23, &proc, ipcproc);
			sem_post(semaphor.semaphore_out);
			--ipcproc->molecule_counter_h;			
			sem_post(semaphor.sem_ready);

			exit(0);
		}
		else if (proc.Elem.symbol == 'O')
		{
			sem_wait(semaphor.semaphore_O);
			++ipcproc->IDMolecule;
			sem_wait(semaphor.semaphore_out);
			GlobalStatus(22, &proc, ipcproc);
			sem_post(semaphor.semaphore_out);			
			sem_post(semaphor.semaphore_H);
			sem_post(semaphor.semaphore_H);
			usleep(arguments.TB > 0 ? rand() % (arguments.TB) * 1000 : 0);			
			sem_wait(semaphor.sem_creating);
			sem_wait(semaphor.semaphore_out);
			GlobalStatus(23, &proc, ipcproc);
			sem_post(semaphor.semaphore_out);
			sem_wait(semaphor.sem_ready);
			sem_wait(semaphor.sem_ready);

			if (arguments.Count_O == ipcproc->IDMolecule)
				sem_post(semaphor.sem__stop);
			sem_post(semaphor.semaphore_O);
			exit(0);
		}
	}		
	while (wait(NULL) > 0);
	Prev(&semaphor);
	FreeMemoryFromQueue(ipcproc);

	return 0;
}
