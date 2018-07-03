
import sys
from task import Task, Scheduler
from schedulers import *

# Constants
RMPA_STR = 'RMPA'
EDF_STR = 'EDF'
LLF_STR = 'LLF'
SCHED_STR = 'SCHED'
OPCE_STR = 'OPCE'

if(len(sys.argv) <= 1):
    print("Please provide input file!")
    exit(1)
elif(len(sys.argv) > 2):
    print("Too many arguments provided!")
    exit(1)

filename = sys.argv[1]


# Read lines from input file
with open(filename, 'r') as file:
    lines = file.readlines()

# There has to be at least 2 lines in the input file
if(len(lines) < 2):
    print("Invalid file...")
    exit(1)

# Split the first line and check simulator metadata
#print(lines[0].split(' '))
info_split = lines[0].split()

# Parse task data from input file
tasks = list()
for i in range(1, len(lines)):
    #print(lines[i])
    if info_split[0] == OPCE_STR:
        task_split = lines[i].split(' ')
        task_id = int(task_split[0])
        task_duration = int(task_split[1])
        task = Task(task_id, 10000, task_duration)
        task.next_start = 100000
        new_list = list()
        for k in range(2, len(task_split)):
            new_list.append(int(task_split[k]))
        task.wait = new_list
    else:
        task_split = lines[i].split(' ')
        task_period = int(task_split[0])
        task_duration = int(task_split[1])
        if info_split[0] == SCHED_STR:
            task_prio = int(task_split[2])
            task_sched_type = task_split[3].rstrip()
            task = Task(i, task_period, task_duration, task_prio, task_sched_type)
            if task_sched_type == "RR":
                task.rr_remaining = int(info_split[2])
                task.rr_period = task.rr_remaining
        else:
            task = Task(i, task_period, task_duration)
    tasks.append(task)
    #print(task_period.__str__() + ":" + task_duration.__str__())

scheduler = Scheduler(tasks)

if info_split[0] == LLF_STR or info_split[0] == SCHED_STR:
    task_period = int(info_split[2])
    scheduler.period = task_period

#
if(info_split[0] == RMPA_STR):
    print("RMPA Mode")
    rmpa_duration = int(info_split[1])
    # Run RMPA simulation
    rmpa(rmpa_duration, scheduler)

elif(info_split[0] == EDF_STR):
    print("EDF Mode")
    edf_duration = int(info_split[1])
    # Run EDF simulation
    edf(edf_duration, scheduler)


elif(info_split[0] == LLF_STR):
    print("LLF Mode")
    llf_duration = int(info_split[1])
    llf_period = int(info_split[2])
    # Run LLF simulation
    llf(llf_duration, llf_period, scheduler)

elif(info_split[0] == SCHED_STR):
    print("SCHED Mode")
    # Run SCHED simulation
    sched_duration = int(info_split[1])
    sched_period = int(info_split[2])
    # Run LLF simulation
    sched(sched_duration, sched_period, scheduler)
elif(info_split[0] == OPCE_STR):
    opce(160, scheduler)
    """
    for task in tasks:
        print(task)
        for w in task.wait:
            print(w)
    """


else:
    print("Mode not supported!")
    exit(1)

print("Simulation Done!\n")
