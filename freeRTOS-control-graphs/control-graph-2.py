#!/usr/bin/env python3
import os
import re
import graphviz

script_dir = os.path.dirname(os.path.abspath(__file__))
app_dir = os.path.abspath(os.path.join(script_dir, '..', 'src', 'app'))
    
def extract_freertos_elements(file_path, tasks, events):
    """
    Extract FreeRTOS tasks and event group operations from the given file.
    This function looks for:
      - Task creation: xTaskCreate(function, "TaskName", ...)
      - Function definitions (to track the current task's function)
      - Event group creation: var = xEventGroupCreate();
      - Event group operations: set, clear, and wait, with event bits extracted.
    """
    current_function = None
    try:
        with open(file_path, 'r', encoding="utf-8") as f:
            for line in f:
                # Check for function definitions (a very basic heuristic)
                # e.g., "void TaskA(void *pvParameters) {"
                func_def_match = re.search(r'^\s*\w+\s+(\w+)\s*\(.*\)\s*{', line)
                if func_def_match:
                    current_function = func_def_match.group(1)
                
                # Extract task creation: xTaskCreate(task_func, "TaskName", ...)
                task_match = re.search(r'xTaskCreate\(\s*(\w+)\s*,\s*"(\w+)"', line)
                if task_match:
                    func, task_name = task_match.groups()
                    tasks[task_name] = func  # store the underlying function

                # Extract event group creation: EventGroupHandle_t var = xEventGroupCreate();
                event_create_match = re.search(r'(\w+)\s*=\s*xEventGroupCreate\(\)', line)
                if event_create_match:
                    event_name = event_create_match.group(1)
                    if event_name not in events:
                        events[event_name] = {"setters": set(), "clears": set(), "waiters": set()}

                # Extract event group set: xEventGroupSetBits(event, BIT_0 | BIT_1);
                event_set_match = re.search(r'xEventGroupSetBits\(\s*(\w+)\s*,\s*([^,)]+)', line)
                if event_set_match:
                    event_name = event_set_match.group(1)
                    bits = event_set_match.group(2).strip()
                    if event_name in events and current_function:
                        events[event_name]["setters"].add((current_function, bits))

                # Extract event group clear: xEventGroupClearBits(event, BIT_0);
                event_clear_match = re.search(r'xEventGroupClearBits\(\s*(\w+)\s*,\s*([^,)]+)', line)
                if event_clear_match:
                    event_name = event_clear_match.group(1)
                    bits = event_clear_match.group(2).strip()
                    if event_name in events and current_function:
                        events[event_name]["clears"].add((current_function, bits))

                # Extract event group wait: xEventGroupWaitBits(event, BIT_0, ...);
                event_wait_match = re.search(r'xEventGroupWaitBits\(\s*(\w+)\s*,\s*([^,)]+)', line)
                if event_wait_match:
                    event_name = event_wait_match.group(1)
                    bits = event_wait_match.group(2).strip()
                    if event_name in events and current_function:
                        events[event_name]["waiters"].add((current_function, bits))
    except Exception as e:
        print(f"Error processing {file_path}: {e}")

def walk_and_extract(directory):
    """
    Recursively walk through the given directory.
    Print each folder and its files.
    Process only files ending in .c or .h and extract FreeRTOS elements.
    """
    tasks = {}
    events = {}
    print(f"Scanning directory: {directory}")
    for root, dirs, files in os.walk(directory):
        print(f"\n📂 Directory: {root}")
        for file in files:
            print(f"  ├── {file}")
            if file.endswith(('.c', '.h')):
                file_path = os.path.join(root, file)
                extract_freertos_elements(file_path, tasks, events)
    return tasks, events

def generate_graph(tasks, events, output_filename="freertos_event_flow"):
    """
    Generate a Graphviz diagram showing:
      - Task nodes (boxes)
      - Event group nodes (dashed ellipses)
      - Edges with labels that include the event bits:
         • Red: tasks that set event bits (e.g. "sets: BIT_0")
         • Orange: tasks that clear event bits (e.g. "clears: BIT_0")
         • Blue: tasks that wait for event bits (e.g. "waits for: BIT_0")
    The diagram is saved as a PNG file.
    """
    dot = graphviz.Digraph(comment='FreeRTOS Event Group Interactions')
    dot.attr(dpi='300')  # Increase resolution
    dot.engine = "dot"

    # Add task nodes (displayed as boxes)
    for task in tasks:
        dot.node(task, task, shape='box')

    # Add event group nodes (displayed as dashed ellipses)
    for event, actions in events.items():
        dot.node(event, event, shape='ellipse', style="dashed")

        # Edges for tasks setting event bits (red)
        for caller, bits in actions["setters"]:
            dot.edge(caller, event, label=f"sets: {bits}", color="red")

        # Edges for tasks clearing event bits (orange)
        for caller, bits in actions["clears"]:
            dot.edge(caller, event, label=f"clears: {bits}", color="orange")

        # Edges for tasks waiting for event bits (blue)
        for caller, bits in actions["waiters"]:
            dot.edge(event, caller, label=f"waits for: {bits}", color="blue")

    output_path = dot.render(output_filename, format='png', cleanup=False)
    print(f"\nGraph saved as {output_path}")
    return output_path

def main():
    # Determine the application directory.
    # For example, if this script is at "/project/scripts/control-graph.py"
    # and you want to analyze "/project/src/app":

    print(f"Application Directory: {app_dir}")

    # Walk the application directory and extract FreeRTOS elements
    tasks, events = walk_and_extract(app_dir)

    # Optionally, print the extracted elements for debugging
    print("\nExtracted tasks:")
    for t in tasks:
        print(f"  {t} -> {tasks[t]}")
    print("\nExtracted events:")
    for event, actions in events.items():
        print(f"  {event}:")
        print(f"    setters: {actions['setters']}")
        print(f"    clears: {actions['clears']}")
        print(f"    waiters: {actions['waiters']}")

    # Generate and save the Graphviz diagram
    generate_graph(tasks, events, os.path.join(script_dir,'freertos_project_flow'))

if __name__ == '__main__':
    main()
