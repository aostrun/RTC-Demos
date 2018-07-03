from task import Task, Scheduler


def rmpa(duration, sched):
    if not isinstance(sched, Scheduler):
        return

    #done = 0
    time = 0
    while 1:

        if time >= duration:
            break
        ready_tasks = sched.get_ready_tasks()
        #if len(ready_tasks) == 0:
         #   break

        ready_tasks = [task for task in ready_tasks if task.done == 0 or time == 0]
        ready_tasks.sort(key=lambda x: x.get_duration())

        #if len(ready_tasks) == 0:
        #    break;

        if len(ready_tasks) > 0:
            min_duration = ready_tasks[0].remaining_time
            if sched.active_task == None or sched.active_task.get_duration() > ready_tasks[0].duration:
                for task in sched.ready_tasks:
                    if task.duration < ready_tasks[0].duration and\
                            task.next_start < (sched.current_time + min_duration):

                        min_duration = task.next_start - sched.current_time

                sched.set_active_task(ready_tasks[0])

        if sched.active_task != None:
            sched.run(min_duration)
            time = sched.current_time
        else:
            print("{}: Empty".format(time+1))
            print("-----------------------------------------------")
            sched.current_time += 1
            time = sched.current_time
            sched.reactivate_tasks()



def edf(duration, sched):
    # Earliest deadline first
    if not isinstance(sched, Scheduler):
        return

    time = 0
    while 1:

        if time >= duration:
            break
        ready_tasks = sched.get_ready_tasks()
        #if len(ready_tasks) == 0:
         #   break

        #ready_tasks = [task for task in ready_tasks if task.done == 0 or time == 0 or (time % task.get_period()) == 0]
        ready_tasks = [task for task in ready_tasks if task.done == 0]
        ready_tasks.sort(key=lambda x:  x.period - (sched.current_time % x.period) )

        #if len(ready_tasks) == 0:
        #    break

        if sched.active_task != None:
            min_duration = sched.active_task.remaining_time
            min_task = sched.active_task
        elif len(ready_tasks) > 0:
            min_duration = ready_tasks[0].remaining_time
            min_task = ready_tasks[0]
            sched.set_active_task(min_task)

        #if sched.active_task == None or\
        #        (sched.active_task.next_start + sched.active_task.period) > (ready_tasks[0].next_start + ready_tasks[0].period) :
        for task in sched.ready_tasks:
            if  sched.current_time > 0 and\
                    task.next_start >= sched.current_time and\
                    task.next_start < sched.current_time + min_task.remaining_time and \
                    task.next_start + task.period < (min_task.next_start + min_task.period):
                min_duration = task.next_start - sched.current_time
                min_task = task

        if(min_duration == 0):
            sched.set_active_task(min_task)
            min_duration = min_task.remaining_time

        if sched.active_task != None:
            sched.run(min_duration)
            time = sched.current_time
        else:
            print("{}: Empty".format(time+1))
            print("-----------------------------------------------")
            sched.current_time += 1
            time = sched.current_time
            sched.reactivate_tasks()


def llf(duration, period, sched):
    # Earliest deadline first
    if not isinstance(sched, Scheduler):
        return

    time = 0
    remaining_period = period
    while 1:

        if time >= duration:
            break

        ready_tasks = sched.get_ready_tasks()
        if len(ready_tasks) == 0:
            break

        if remaining_period == period:
            # ready_tasks = [task for task in ready_tasks if task.done == 0 or time == 0 or (time % task.get_period()) == 0]
            ready_tasks = [task for task in ready_tasks if task.done == 0 and task.remaining_time > 0]
            ready_tasks.sort(key=lambda x: x.get_laxity(sched.current_time) )


        if len(ready_tasks) > 0:
            if sched.active_task == None:
                for i in range(len(ready_tasks)):
                    if ready_tasks[i].remaining_time > 0:
                        sched.set_active_task(ready_tasks[0])
                        break
            elif ready_tasks[0].get_laxity(sched.current_time) < sched.active_task.get_laxity(sched.current_time):
                sched.set_active_task(ready_tasks[0])




        if sched.active_task != None:
            if sched.active_task.remaining_time < period:
                min_duration = sched.active_task.remaining_time
            else:
                min_duration = period

            sched.run(min_duration)
        else:
            print("{}: Empty".format(time+1))
            print("-----------------------------------------------")
            min_duration = remaining_period
            sched.current_time += period
            sched.reactivate_tasks()

        remaining_period -= min_duration
        if remaining_period <= 0:
            remaining_period = period
        time += min_duration


def sched(duration, period, sched):
    # Earliest deadline first
    if not isinstance(sched, Scheduler):
        return

    time = 0
    remaining_period = period
    while 1:

        if time >= duration:
            break

        ready_tasks = sched.get_ready_tasks()
        if len(ready_tasks) == 0:
            break

        #if remaining_period == period:
        # ready_tasks = [task for task in ready_tasks if task.done == 0 or time == 0 or (time % task.get_period()) == 0]
        ready_tasks = [task for task in ready_tasks if task.done == 0] # and task.skip == 0]
        if len(ready_tasks) == 0:
            ready_tasks = [task for task in sched.get_ready_tasks() if task.done == 0]
        ready_tasks.sort(key=lambda x: x.prio)

        selected_task = sched.active_task
        if len(ready_tasks) > 0:
            for task in ready_tasks:
                if selected_task == None:
                    selected_task = task
                elif selected_task.skip == 1 and task.prio == selected_task.prio:
                    selected_task = task
                elif task.prio < selected_task.prio and selected_task.skip == 1:
                    selected_task = task

            if selected_task != None and selected_task != sched.active_task:
                sched.set_active_task(selected_task)
            """
            if sched.active_task == None or sched.active_task.prio > ready_tasks[0].prio:
                sched.set_active_task(ready_tasks[0])
            elif sched.active_task.prio == "RR":
                sched.set_active_task(ready_tasks[0])
            """

        if sched.active_task != None:
            if sched.active_task.remaining_time < remaining_period:
                min_duration = sched.active_task.remaining_time
            else:
                min_duration = remaining_period

            if sched.active_task.rr_remaining > 0 and min_duration > sched.active_task.rr_remaining:
                min_duration = sched.active_task.rr_remaining


            sched.run(min_duration)

        else:
            print("{}: Empty".format(time + 1))
            print("-----------------------------------------------")
            min_duration = remaining_period
            sched.current_time += period
            sched.reactivate_tasks()

        remaining_period -= min_duration
        if remaining_period <= 0:
            remaining_period = period
        time += min_duration


def opce(duration, sched):
    # Earliest deadline first
    if not isinstance(sched, Scheduler):
        return

    time = 0
    done = list()
    while 1:

        if time >= duration:
            break

        ready_tasks = sched.get_ready_tasks()
        if len(ready_tasks) == 0:
            break

        # if remaining_period == period:
        # ready_tasks = [task for task in ready_tasks if task.done == 0 or time == 0 or (time % task.get_period()) == 0]
        ready_tasks = [task for task in ready_tasks if task.done == 0]  # and task.skip == 0]

        selected_task = sched.active_task
        if len(ready_tasks) > 0:
            for task in ready_tasks:
                start = 1
                for w in task.wait:
                    if w == 0 or done.__contains__(w):
                        continue
                    else:
                        print(str(task) + " ceka: " + str(w))
                        start = 0
                        break
                if start == 0:
                    continue
                else:
                    selected_task = task
                    break


            if selected_task != None and selected_task != sched.active_task:
                sched.set_active_task(selected_task)

        if sched.active_task != None:
            min_duration = 1
            sched.run(min_duration)

        else:
            print("{}: Empty".format(time + 1))
            print("-----------------------------------------------")
            #sched.reactivate_tasks()

        if sched.active_task != None and sched.active_task.done == 1:
            done.append(sched.active_task.id)
            sched.ready_tasks.append(sched.active_task)
            sched.active_task = None

        time += min_duration