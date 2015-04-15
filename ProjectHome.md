**Note: the fiber framework isn't ready for use at the moment**

The fiber framework is intended for programming of high-performance cross-platform network applications, where linear execution flow is preferred comparing to event-driven execution flow due to simplicity. The framework allows to almost seamlessly substitute usual threads by userspace threads without changing application's architecture.

The framework is written in C. It uses only those libraries, which are included in standard OS installations, so it is self-contained and doesn't require external tools and libraries.

# Advantages #

## Comparing to OS thread model ##
  * There is no need to enter kernel space while switching between execution threads (fibers in our case). This can significantly reduce CPU overhead required for switching between fibers comparing to usual threads.

  * There is no need to use most of synchronization primitives in order to avoid race conditions, which are specific to multithreaded programming, because all fibers are executed in the same thread.

## Comparing to event-driven model ##
The fiber framework allows to keep the main thread model advantage comparing to event-driven model - linear execution flow, which is easier to program and understand comparing to execution flow driven by events (There are tasks, which are naturally suited to event-driven model - GUI applications, for example. Don't use fiber framework for these tasks ;) ).

# Disadvantages #

## Comparing to OS thread model ##
  * The currently executing fiber can switch to another fiber, which is ready for execution, only by calling functions, which can potentially block - IO or synchronization functions. This means that the following code, which is executed in the fiber context, is equivalent to deadlock for the whole fiber framework:
```
for (;;)
{
  /* do anything, but don't call 'break',
   * any IO or synchronization function
   */
}
```
> However, such a code usually indicates that the application was pourly designed and potentially contains performance or deadlock bugs.

  * If the currently executing fiber accesses memory, which was swapped out to disk, **all** fibers will wait while the memory will be loaded back into physical memory.

  * The OS process driven by fiber framework doesn't scale on multi-CPU systems. This 'disadvantage' was introduced intentionally in order to encourage the ideal scalability method - 'share nothing' - just start as many OS processes driven by fiber framework as the number of CPUs in the system. Then the OS automatically (and optimally in most cases) will distribute corresponding processes among available CPUs. This also increase reliability of the fiber framework driven application, because other processes won't suffer if one process will crash.

## Comparing to event-driven model ##
In the fiber framework every execution thread (fiber) requires at least one page of memory for stack, while event-based model requires only sizeof(state) bytes of memory per execution thread, where state is the execution thread state.
This means that the 10K execution threads will occupy at least 40Mb of memory for stacks, which isn't very much for the current RAM prices ;)

# Functionality #
The fiber framework provides the following functionality:
  * Fibers' management. Functions in this group are semantically identical to usual thread management functions: create, start, join, delete.

  * Execution blocking, fiber-incompatible or CPU-intensive functions in background thread from worker thread pool. This allows to use with ease third-party fiber-unaware libraries.

  * Scheduling tasks for execution in the worker fiber pool. This is semantically identical to scheduling tasks for execution in the usual worker thread pool.

  * Inter-fiber synchronization - event and lock. The event is semantically identical to the event provided by WinAPI CreateEvent() function. The lock is semantically identical to windows' critical section or pthread's mutex. It is used for data access synchronization in the code, which can potentially block. For instance:
```
/* call the callback for each entry in the list
 * The callback can call IO or synchronization function,
 * i.e. it can block.
 * We should use lock here, because the current fiber
 * can switch to another fiber when calling blocking function
 * in the callback. And that fiber can call modify_list()
 * function, which will delete the current_entry.
 */
void for_each_entry(list *list, void (*callback)(list_entry *))
{
  lock(list->lock);
  list_entry *current_entry = list->head;
  while (current_entry != NULL)
  {
    callback(current_entry);
    current_entry = current_entry->next;
  }
  unlock(list->lock);
}

/* add or remove elements from the list
 * We should use lock here even if there are no blocking
 * functions between lock() and unlock(),
 * because this function can be called at the moment,
 * when another fiber is blocked in the for_each_entry()
 * function.
 */
void modify_list(list *list)
{
  lock(list->lock);
  /* modify list here */
  unlock(list->lock);
}
```
> Note that there is no need to use the lock in the code, which reads/writes the given data from different fibers, but **never calls blocking functions** during access to the data.

  * Inter-fiber synchronization - blocking queue. Is is intended for use in producer-consumer applications.

  * Network functions for working with TCP and UDP protocols, which are semantically identical to usual blocking functions from BSD socket API: create, close, connect, accept, read, write, shutdown.

  * File IO functions, which are semantically identical to usual blocking functions from stdio.h: open, close, read, write, seek.