import os
import re
import graphviz

tasks = {}
events = {}
queues = {}
semaphores = {}
event_bits = {}  # Store event bit definitions
task_relationships = {}  # Store task relationships based on event bits
script_dir = os.path.dirname(os.path.abspath(__file__))
application_directory = os.path.abspath(os.path.join(script_dir, '..', 'src', 'app')) 
    
def extract_event_bits(file_path):
    with open(file_path, 'r', encoding="utf-8") as file:
        content = file.read()
        # Match event bit definitions like #define SOME_BIT (1 << 0)
        bit_matches = re.finditer(r'#define\s+(\w+_BIT)\s+\(1\s*<<\s*(\d+)\)', content)
        for match in bit_matches:
            bit_name, bit_position = match.groups()
            event_bits[bit_name] = int(bit_position)

def extract_freertos_elements(file_path, tasks, events, queues, semaphores):
    with open(file_path, 'r', encoding="utf-8") as file:
        content = file.read()
        
        # First pass: Find all task function definitions
        task_func_matches = re.finditer(r'void\s+(\w+)\s*\([^)]*\)\s*{', content)
        for match in task_func_matches:
            func_name = match.group(1)
            tasks[func_name] = {
                'function': func_name,
                'sets_bits': set(),
                'clears_bits': set(),
                'waits_bits': set(),
                'task_name': ''  # Will be filled when we find xTaskCreate
            }

        # Second pass: Match task creation and link to function definitions
        task_creates = re.finditer(r'xTaskCreate\s*\(\s*(\w+)\s*,\s*"([^"]+)"', content)
        for match in task_creates:
            func_name, task_name = match.groups()
            if func_name in tasks:
                tasks[func_name]['task_name'] = task_name
            else:
                print(f"Warning: Task function {func_name} not found but created with name {task_name}")

        # Third pass: Extract event group operations for each task
        for func_name, task_info in tasks.items():
            # Find the task function definition and its content
            func_pattern = re.escape(func_name) + r'\s*\([^)]*\)\s*{([^}]+)}'
            func_matches = re.finditer(func_pattern, content, re.DOTALL)
            
            for func_match in func_matches:
                func_content = func_match.group(1)
                
                # Find event bit operations with improved patterns
                set_bits = re.finditer(r'xEventGroupSetBits\s*\([^,]+,\s*([^)]+)\)', func_content)
                clear_bits = re.finditer(r'xEventGroupClearBits\s*\([^,]+,\s*([^)]+)\)', func_content)
                wait_bits = re.finditer(r'xEventGroupWaitBits\s*\([^,]+,\s*([^,]+)', func_content)

                # Extract and clean bit names
                for match in set_bits:
                    bits = match.group(1).strip()
                    task_info['sets_bits'].update(bit for bit in bits.split('|') if '_BIT' in bit)
                
                for match in clear_bits:
                    bits = match.group(1).strip()
                    task_info['clears_bits'].update(bit for bit in bits.split('|') if '_BIT' in bit)
                
                for match in wait_bits:
                    bits = match.group(1).strip()
                    task_info['waits_bits'].update(bit for bit in bits.split('|') if '_BIT' in bit)

def create_task_graph():
    dot = graphviz.Digraph(comment='FreeRTOS Task Relationships')
    dot.attr(rankdir='LR')

    # Create nodes for tasks with actual task names
    for func_name, task_info in tasks.items():
        if task_info['task_name']:  # Only include tasks that were actually created
            node_name = task_info['task_name']
            label = f"{node_name}\\n({func_name})\\n"
            
            if task_info['sets_bits']:
                label += f"Sets: {', '.join(task_info['sets_bits'])}\\n"
            if task_info['clears_bits']:
                label += f"Clears: {', '.join(task_info['clears_bits'])}\\n"
            if task_info['waits_bits']:
                label += f"Waits: {', '.join(task_info['waits_bits'])}"
            
            dot.node(node_name, label)

    # Create edges between tasks based on event bits
    for func1, task1_info in tasks.items():
        if not task1_info['task_name']:
            continue
        for func2, task2_info in tasks.items():
            if not task2_info['task_name'] or func1 == func2:
                continue
                
            # If task1 sets bits that task2 waits for
            common_bits = task1_info['sets_bits'].intersection(task2_info['waits_bits'])
            if common_bits:
                dot.edge(task1_info['task_name'], task2_info['task_name'],
                        label=f"Sets: {', '.join(common_bits)}")

            # If task1 clears bits that task2 waits for
            common_bits = task1_info['clears_bits'].intersection(task2_info['waits_bits'])
            if common_bits:
                dot.edge(task1_info['task_name'], task2_info['task_name'],
                        label=f"Clears: {', '.join(common_bits)}",
                        style='dashed')

    return dot

def extract_freertos_elements_from_directory(directory):
    global tasks, events, queues, semaphores, event_bits

    print("Walking through directory...")
    for root, _, files in os.walk(directory):
        print(f"📂 Directory: {root}")
        for file in files:
            if file.endswith((".c", ".h")):
                file_path = os.path.join(root, file)
                print(f"  ├── Parsing: {file}")
                extract_event_bits(file_path)
                extract_freertos_elements(file_path, tasks, events, queues, semaphores)

    # Create and save the task relationship graph
    graph = create_task_graph()
    graph.render(os.path.join(script_dir,"freertos_task_relationships"), format="png", cleanup=True)
    print(graph.source)
    print("Task relationship graph has been generated as 'freertos_task_relationships.png'")

    return tasks, events, queues, semaphores

if __name__ == "__main__":

    print(application_directory)
    tasks, events, queues, semaphores = extract_freertos_elements_from_directory(application_directory)  # Change this