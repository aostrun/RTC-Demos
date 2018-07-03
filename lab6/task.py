
class Task:

    id = 0
    period = 0
    duration = 0
    remaining_time = 0
    done = 0
    next_start = 0
    prio = 0
    sched_type = None
    rr_remaining = 0
    rr_period = 0
    skip = 0
    wait = list()

    def __init__(self,_id, _period, _duration, _prio=0, _sched_type=None, _rr_time=0):
        self.id = _id
        self.period = _period
        self.duration = _duration
        self.remaining_time = self.duration
        self.prio = _prio
        self.done = 0
        self.next_start = 0
        self.sched_type = _sched_type
        self.rr_remaining = _rr_time

    def get_id(self):
        return self.id

    def get_duration(self):
        return self.duration

    def get_period(self):
        return self.period

    def get_remaining_time(self):
        return self.remaining_time

    def get_sched_type(self):
        return self.sched_type

    def reset_done(self):
        self.rr_remaining = self.rr_period
        self.remaining_time = self.duration
        self.done = 0

    def get_laxity(self, time):
        return (self.next_start + self.period) - (time + self.remaining_time)

    def elapsed(self):
        return (self.duration - self.remaining_time)

    def run(self, time):
        #if self.remaining_time == self.duration:
            #self.next_start += self.period
        self.remaining_time -= time
        if self.remaining_time <= 0:
            self.remaining_time = 0
            self.done = 1
            self.next_start += self.period

            return 1
        #else:
        #    if self.sched_type == "RR":
        #        self.skip = 1

        return 0

    def __str__(self):
        return "" + str(self.id) +  ": [" + str(self.period) + ", " \
                + str(self.duration) + "]"

    def __unicode__(self):
        return self.__str__()


class Scheduler:

    active_task = None
    ready_tasks = list()
    current_time = 0
    period = 0

    def __init__(self, tasks, period=0):
        if not isinstance(tasks, list):
            return
        self.ready_tasks = tasks
        self.current_time = 0
        self.period = period

    def get_active_task(self):
        return self.active_task

    def get_ready_tasks(self):
        return self.ready_tasks

    def set_active_task(self, task):
        if(task not in self.ready_tasks):
            print("Task is not ready!")
            return

        if self.active_task != None:
            self.ready_tasks.append(self.active_task)

        self.active_task = task
        self.ready_tasks.remove(task)

    def reactivate_tasks(self):
        # Reactive tasks by their period
        for task in self.ready_tasks:
            # Reactive tasks
            if task.done == 1 and task.next_start <= self.current_time:
                task.reset_done()
            # Check for RR tasks
            if task.sched_type == "RR" and task.skip == 1:
                task.skip = 0

    def run(self, time):
        if self.active_task == None:
            return
        if( not isinstance(self.active_task, Task)):
            return

        done = self.active_task.run(time)
        self.current_time += time
        self.active_task.rr_remaining -= time
        #print(str(self.current_time) + ": " + str(self.active_task))
        print("Vrijeme: {}s".format(self.current_time))
        if (self.active_task.remaining_time == 0):
            print("Događaj: zadatak {} gotov".format(self.active_task.id))
        elif (self.active_task.sched_type == None and self.active_task.remaining_time > 0):
            print("Događaj: zadatak {} zamijenjen".format(self.active_task.id))
        elif(self.active_task.sched_type == 'RR' and self.active_task.rr_remaining <= 0):
            print("Događaj: zadatak {} RR timeout".format(self.active_task.id))
        print("Task: {}".format(self.active_task))
        print("Pripravni: {}".format([str(task) for task in self.ready_tasks if task.done == 0]))

        print("-----------------------------------------------")
        self.reactivate_tasks()


        if self.active_task.sched_type == "RR" and self.active_task.rr_remaining <= 0:
            if done != 1 and self.active_task.sched_type == "RR":
                self.active_task.skip = 1
                if self.active_task.remaining_time > 0:
                    if self.active_task.remaining_time < self.active_task.rr_period:
                        self.active_task.rr_remaining = self.active_task.remaining_time
                    else:
                        self.active_task.rr_remaining = self.active_task.rr_period
        if done == 1:
            self.ready_tasks.append(self.active_task)
            #self.active_task = None
