import os
import re
import graphviz

tasks = {}
events = {}
queues = {}
semaphores = {}
script_dir = os.path.dirname(os.path.abspath(__file__))
application_directory = os.path.abspath(os.path.join(script_dir, '..', 'src', 'app')) 

def extract_freertos_elements_from_directory(directory):
    global tasks
    global events
    global queues 
    global semaphores

    print("walking through directory...")
    # Scan all .c and .h files in the directory
    for root, _, files in os.walk(directory):
        print(f"📂 Directory: {root}")  # Print folder name
        for file in files:
            print(f"  ├── {file}")  # Print files inside the folder
            if file.endswith(".c") or file.endswith(".h"):
                file_path = os.path.join(root, file)
                print(f"Parsing: {file_path}")  # Debugging
                extract_freertos_elements(file_path, tasks, events, queues, semaphores)

    return tasks, events, queues, semaphores

def extract_freertos_elements(file_path, tasks, events, queues, semaphores):
    with open(file_path, 'r', encoding="utf-8") as file:
        for line in file:
            # Match task creation
            task_match = re.search(r'xTaskCreate\((\w+),\s*"(\w+)"', line)
            if task_match:
                func, task_name = task_match.groups()
                tasks[task_name] = func

            # Match event group creation
            event_create_match = re.search(r'(\w+)\s*=\s*xEventGroupCreate\(\)', line)
            if event_create_match:
                event_name = event_create_match.group(1)
                events[event_name] = {"setters": set(), "waiters": set()}

            # Match event group setting and waiting
            event_set_match = re.search(r'xEventGroupSetBits\((\w+),', line)
            event_wait_match = re.search(r'xEventGroupWaitBits\((\w+),', line)
            if event_set_match:
                event_name = event_set_match.group(1)
                if event_name in events:
                    events[event_name]["setters"].add(line.strip())
            if event_wait_match:
                event_name = event_wait_match.group(1)
                if event_name in events:
                    events[event_name]["waiters"].add(line.strip())

            # Match queue creation
            queue_create_match = re.search(r'(\w+)\s*=\s*xQueueCreate\(\d+,\s*\d+\)', line)
            if queue_create_match:
                queue_name = queue_create_match.group(1)
                queues[queue_name] = {"senders": set(), "receivers": set()}

            # Match queue sending and receiving
            queue_send_match = re.search(r'xQueueSend\((\w+),', line)
            queue_receive_match = re.search(r'xQueueReceive\((\w+),', line)
            if queue_send_match:
                queue_name = queue_send_match.group(1)
                if queue_name in queues:
                    queues[queue_name]["senders"].add(line.strip())
            if queue_receive_match:
                queue_name = queue_receive_match.group(1)
                if queue_name in queues:
                    queues[queue_name]["receivers"].add(line.strip())

            # Match semaphore creation, giving, and taking
            semaphore_create_match = re.search(r'(\w+)\s*=\s*xSemaphoreCreate\w+\(\)', line)
            semaphore_give_match = re.search(r'xSemaphoreGive\((\w+)\)', line)
            semaphore_take_match = re.search(r'xSemaphoreTake\((\w+),', line)
            if semaphore_create_match:
                sem_name = semaphore_create_match.group(1)
                semaphores[sem_name] = {"givers": set(), "takers": set()}
            if semaphore_give_match:
                sem_name = semaphore_give_match.group(1)
                if sem_name in semaphores:
                    semaphores[sem_name]["givers"].add(line.strip())
            if semaphore_take_match:
                sem_name = semaphore_take_match.group(1)
                if sem_name in semaphores:
                    semaphores[sem_name]["takers"].add(line.strip())

def generate_graph(tasks, events, queues, semaphores):
    dot = graphviz.Digraph(comment='FreeRTOS Task & Communication Flow')

    # Add tasks
    for task, func in tasks.items():
        dot.node(task, task, shape='box')

    # Add event groups and their interactions
    for event, interactions in events.items():
        dot.node(event, event, shape='ellipse', style="dashed")

        for setter in interactions["setters"]:
            match = re.search(r'(\w+)\s*\(', setter)
            if match:
                setter_task = match.group(1)
                dot.edge(setter_task, event, label="sets", color="red")

        for waiter in interactions["waiters"]:
            match = re.search(r'(\w+)\s*\(', waiter)
            if match:
                waiter_task = match.group(1)
                dot.edge(event, waiter_task, label="waits for", color="blue")

    # Add queues
    for queue, interactions in queues.items():
        dot.node(queue, queue, shape='diamond', style="filled", fillcolor="lightgray")

        for sender in interactions["senders"]:
            match = re.search(r'(\w+)\s*\(', sender)
            if match:
                sender_task = match.group(1)
                dot.edge(sender_task, queue, label="sends to", color="green")

        for receiver in interactions["receivers"]:
            match = re.search(r'(\w+)\s*\(', receiver)
            if match:
                receiver_task = match.group(1)
                dot.edge(queue, receiver_task, label="received by", color="purple")

    # Add semaphores
    for sem, interactions in semaphores.items():
        dot.node(sem, sem, shape='parallelogram', style="filled", fillcolor="yellow")

        for giver in interactions["givers"]:
            match = re.search(r'(\w+)\s*\(', giver)
            if match:
                giver_task = match.group(1)
                dot.edge(giver_task, sem, label="gives", color="orange")

        for taker in interactions["takers"]:
            match = re.search(r'(\w+)\s*\(', taker)
            if match:
                taker_task = match.group(1)
                dot.edge(sem, taker_task, label="takes", color="brown")

    # Render graph
    dot.render(os.path.join(script_dir,'freertos_project_flow'), format='png', cleanup=True)
    print(dot.source)
# Example usage

if __name__ == "__main__":

    print(application_directory)
    tasks, events, queues, semaphores = extract_freertos_elements_from_directory(application_directory)  # Change this
    generate_graph(tasks, events, queues, semaphores)
