from math import pi, cos, sin
import tkinter as tk
from tkinter import ttk
from PIL import Image, ImageTk
import numpy as np
import subprocess
import sys

file = "./bin/raycast"
maze_file = "data/maze.txt"
if len(sys.argv) > 1:
    maze_file = sys.argv[1]
texture_file = "data/wolftextures.ppm"
texture_count_x = 8
texture_count_y = 1
if len(sys.argv) > 4:
    texture_file = sys.argv[2]
    texture_count_x = int(sys.argv[3])
    texture_count_y = int(sys.argv[4])

# get start point py running with maze -I
result = subprocess.run([file, maze_file, "-I"], capture_output=True, text=True)
lines = result.stdout.splitlines()
start_line = next(line for line in lines if line.startswith("Start point:"))
start_coords = start_line.split(":")[1].strip().strip("()").split(",")
start_x = float(start_coords[0])+0.5
start_y = float(start_coords[1])+0.5
# print(f"Start position from C program: ({start_x}, {start_y})")

class Point:
    def __init__(self, x, y):
        self.x = x
        self.y = y

class GameState:
    def __init__(self, start_x, start_y):
        self.position = Point(start_x, start_y)
        self.angle = 0.0  # radians
        self.fov = 60.0
        self.move_speed = 0.1
        self.rotate_speed = 0.1  # radians per keypress
        self.width = 800
        self.height = 600

game_state = GameState(start_x, start_y)

# Create GUI
root = tk.Tk()
root.title("Maze Raycaster - WASD Controls")
root.geometry(f"{game_state.width}x{game_state.height}")

# Create canvas
canvas = tk.Canvas(root, width=game_state.width, height=game_state.height, bg="black")
canvas.pack()

# Create photo image placeholder
photo = None

# continously running raycaster subprocess
# data/maze.txt -C data/wolftextures.ppm 8 1 output/rendered_maze_texture2.ppm 800 600 60
print("Starting raycaster subprocess...")
program = subprocess.Popen([
    file,
    maze_file,
    "-C",
    texture_file,
    str(texture_count_x),
    str(texture_count_y),
    str(game_state.width),
    str(game_state.height),
    str(int(game_state.fov))
], stdin=subprocess.PIPE, stdout=subprocess.PIPE)


def update_display():
    global photo

    # send angle and position to subprocess
    input_data = f"{game_state.position.x} {game_state.position.y} {game_state.angle * 180.0 / pi : .6f}\n"
    # print(f"Sending to subprocess: {input_data}")
    try: 
        program.stdin.write(input_data.encode())
        program.stdin.flush()
        img = program.stdout.read(game_state.width * game_state.height * 3)

        data_array = np.frombuffer(img, dtype=np.uint8)
        data_array = data_array.reshape((game_state.height, game_state.width, 3))
        # print(f"Received image data of shape: {data_array.shape}")

        pil_img = Image.fromarray(data_array, 'RGB')
        photo = ImageTk.PhotoImage(pil_img)
        canvas.delete("all")
        canvas.create_image(0, 0, anchor="nw", image=photo)
    except BrokenPipeError:
        print(f"Error: Subprocess pipe is broken.")

def move_forward():
    new_x = game_state.position.x + cos(game_state.angle) * game_state.move_speed
    new_y = game_state.position.y + sin(game_state.angle) * game_state.move_speed
    game_state.position.x = new_x
    game_state.position.y = new_y
    update_display()

def move_backward():
    new_x = game_state.position.x - cos(game_state.angle) * game_state.move_speed
    new_y = game_state.position.y - sin(game_state.angle) * game_state.move_speed
    game_state.position.x = new_x
    game_state.position.y = new_y
    update_display()

def move_left():
    new_x = game_state.position.x + cos(game_state.angle - pi/2) * game_state.move_speed
    new_y = game_state.position.y + sin(game_state.angle - pi/2) * game_state.move_speed
    game_state.position.x = new_x
    game_state.position.y = new_y
    update_display()

def move_right():
    new_x = game_state.position.x - cos(game_state.angle - pi/2) * game_state.move_speed
    new_y = game_state.position.y - sin(game_state.angle - pi/2) * game_state.move_speed
    game_state.position.x = new_x
    game_state.position.y = new_y
    update_display()

def rotate_left():
    game_state.angle -= game_state.rotate_speed
    update_display()

def rotate_right():
    game_state.angle += game_state.rotate_speed
    update_display()

# Track pressed keys
pressed_keys = set()

# Movement update loop
def game_loop():
    moved = False
    if 'w' in pressed_keys:
        move_forward()
        moved = True
    if 's' in pressed_keys:
        move_backward()
        moved = True
    if 'a' in pressed_keys:
        rotate_left()
        moved = True
    if 'd' in pressed_keys:
        rotate_right()
        moved = True
    if 'q' in pressed_keys:
        move_left()
        moved = True
    if 'e' in pressed_keys:
        move_right()
        moved = True
    if moved:
        update_display()
    root.after(16, game_loop)  # ~60 FPS

# Key press/release handlers
def on_key_press(event):
    key = event.keysym.lower()
    if key == 'escape':
        root.quit()
    else:
        pressed_keys.add(key)

def on_key_release(event):
    key = event.keysym.lower()
    if key in pressed_keys:
        pressed_keys.remove(key)

# Bind key events
root.bind('<KeyPress>', on_key_press)
root.bind('<KeyRelease>', on_key_release)
root.focus_set()

# Add instructions
instructions = tk.Label(root, text="WASD: Move/Rotate | QE: Strafe | ESC: Exit", 
                       bg="black", fg="white", font=("Arial", 12))
instructions.pack(side="bottom")

# Initial display
update_display()

# Start the game loop
game_loop()

# Handle window close
def on_closing():
    """Clean up resources when closing"""
    # c.free(maze)
    # c.free_texture_data(texture_data)
    program.terminate()
    root.destroy()

root.protocol("WM_DELETE_WINDOW", on_closing)

# Start the GUI
root.mainloop()