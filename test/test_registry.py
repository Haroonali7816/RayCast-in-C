import os
import re
import json
from shutil import which
import textwrap

def is_installed(name):
    """Check whether `name` is on PATH and marked as executable."""
    return which(name) is not None


def source_files(src_dir):
    files = []
    for file in os.listdir(src_dir):
        if (file.endswith(".c") or file.endswith(".h")) and file != "unit_tests.c" and file != "test_main.c" and "gui" not in file:
            files.append(src_dir+"/"+file)
    return files
    
def style_test(tu, test_name):
    src_dir=tu.join_base("src")
    test_bin = ("splint")
    if not is_installed(test_bin):
        return tu.FAILURE("splint binary not available")
    
    args = [
            "-nullstate", "-compmempass", "-unreachable", "-noeffect",
            "-retvalother","-retvalint","-branchstate", "-exportlocal", "-usedef", "-compdef", "-temptrans", "+charint", "-predboolint", "-predboolothers", "-nullret", "-nullderef", "-nullpass", "-unrecog", "+matchanyintegral", "-mustfreefresh", "-formatcode", "-mustdefine", "-initallelements","-boolops", "-compdestroy", "+posixlib", "-unqualifiedtrans", "-shiftnegative", 
    ]
    args += source_files(src_dir)
    
    run_res = tu.run(test_bin, args, timeout_secs=20)
    rc,outs,errs=run_res

    msg = "splint found warnings."
    ret_text=""
    if (len(outs) > 0):
        ret_text += "\nstdout:\n" + textwrap.indent(outs, "  ")
    if (len(errs) > 0):
        ret_text += "\nstderr:\n" + textwrap.indent(errs, "  ")
    if "no warning" not in ret_text:
        return tu.FAILURE(msg+ret_text)
    return tu.check(run_res, "splint test failed")

def style_test_cppcheck(tu, test_name):
    test_bin = ("cppcheck")
    if not is_installed(test_bin):
        return tu.FAILURE("cppcheck binary not available")

    run_res = tu.run("sh", ["-c", "cppcheck "+" ".join(source_files(tu.join_base("src")))], timeout_secs=20)
    rc,outs,errs=run_res

    msg = "cppcheck found warnings."
    ret_text=""
    if (len(outs) > 0):
        ret_text += "\nstdout:\n" + textwrap.indent(outs, "  ")
    if (len(errs) > 0):
        ret_text += "\nstderr:\n" + textwrap.indent(errs, "  ")
    if "error:" in ret_text:
        return tu.FAILURE(msg+ret_text)
    return tu.check(run_res, "cppcheck test failed")

def style_test_cpplint(tu, test_name):
    test_bin = ("cpplint")
    if not is_installed(test_bin):
        return tu.FAILURE("cpplint binary not available")

    run_res = tu.run("sh", ["-c", "cpplint --filter=-build/header_guard,-legal/copyright,-build/include_subdir,-readability/casting,-readability/todo,-whitespace/line_length,-build/include_what_you_use "+ " ".join(source_files(tu.join_base("src")))], timeout_secs=20)
    rc,outs,errs=run_res

    msg = "cpplint found warnings."
    ret_text=""
    if (len(outs) > 0):
        ret_text += "\nstdout:\n" + textwrap.indent(outs, "  ")
    if (len(errs) > 0):
        ret_text += "\nstderr:\n" + textwrap.indent(errs, "  ")
    if "Total errors found" in ret_text:
        return tu.FAILURE(msg+ret_text)
    return tu.check(run_res, "cpplint test failed")

def style_complexity(tu, test_name):
    try:
        src_dir=tu.join_base("src")
        test_bin = ("/home/prog2/.local/bin/metrix++")
        if not is_installed(test_bin):
            return tu.FAILURE("metrix++ binary not available")
        args = ["collect", "--std.code.complexity.cyclomatic", "--db-file=/tmp/data.db"]
        files = source_files(src_dir)
        files = [f for f in files if "gui" not in f]
        args += files
        run_res = tu.run(test_bin, args, timeout_secs=20)
        rc,outs,errs=run_res
        if rc != 0:
            return tu.FAILURE("metrix++ complexity collection failed with error:\n"+errs)
        args = ["export", "--db-file=/tmp/data.db"]
        args += files
        run_res = tu.run(test_bin, args, timeout_secs=20)
        rc,outs,errs=run_res
        if rc != 0:
            return tu.FAILURE("metrix++ complexity export failed with error:\n"+errs)
        complexities = []
        for line in outs.splitlines():
            parts = line.split(",")
            if len(parts) != 7:
                continue
            if parts[2]!="function":
                continue
            complexity = int(parts[6])
            complexities.append((complexity, parts))
        if len(complexities) < 5:
            return tu.FAILURE("metrix++ complexity export did not find enough functions, output:\n"+outs)
        max_complexity = max(complexity for complexity, _ in complexities)
        complexity_threshold = 30
        def display_complexity(complexity, parts):
            file = parts[0].split("/")[-1]
            name = parts[1]
            kind = parts[2]
            line_start = parts[4]
            line_end = parts[5]
            complexity = parts[6]
            return f"{file}:{line_start}-{line_end} {kind} {name} has cyclomatic complexity {complexity}"

        if max_complexity > complexity_threshold:
            report = "\n".join(display_complexity(complexity, parts) for complexity, parts in complexities if complexity > complexity_threshold)
            return tu.FAILURE(f"Complexity exceeds threshold of {complexity_threshold}. Functions exceeding threshold:\n{report}")
        return tu.SUCCESS()
    except Exception as e:
        return tu.FAILURE(f"An error occurred while checking complexity: {str(e)}")



test_data = {}
base_path = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
data_file = os.path.join(base_path, "test/data/tests.json")
with open(data_file) as f:
    raw_test_data = json.load(f)
    for test_name, test in raw_test_data.items():
        test_data[test_name] = test
        test_data[test_name]['args'] = [(os.path.join(base_path, arg) if '/' in arg else arg) for arg in test["args"]]
    del raw_test_data
solver = os.path.join(base_path, "bin/raycast")

def run_raycast(tu, test):
    (rc, outs, errs) = tu.run(solver, test["args"])
    for errline in errs.split('\n'):
        if len(errline) > 0:
            print('DEBUG:', errline)
    return outs


def read_pixels_from_data(tu, test, data):
    if not data:
        return tu.FAILURE(f'Cannot read output file {outfile}.'), None
    width = test["width"]
    height = test["height"]
    expected_header = f'P6\n{width} {height}\n255'
    if not data.startswith(expected_header.encode()):
        return tu.FAILURE(f'Invalid header.'), None
    values = list(data[len(expected_header)+1:])
    if len(values) != width * height * 3:
        return tu.FAILURE(f'Incorrect number of bytes. Expected {width * height * 3}, got {len(values)}.'), None
    pixels = [tuple(values[i:i + 3]) for i in range(0, len(values), 3)]
    return None, pixels


def get_pixels(tu, test):
    run_raycast(tu, test)
    outfile = f'{test["name"]}.ppm'
    if not os.path.exists(outfile):
        return tu.FAILURE(f'Output file {outfile} does not exist.'), None
    data = None
    with open(outfile, 'rb') as f:
        data = f.read()
    os.remove(outfile)
    return read_pixels_from_data(tu, test, data)


def get_reference_pixels(tu, test):
    ref_file = os.path.join(base_path, f'test/data/reference/{test["name"]}.ppm')
    data = None
    with open(ref_file, 'rb') as f:
        data = f.read()
    error, pixels = read_pixels_from_data(tu, test, data)
    assert error is None
    return pixels


def get_neighboring_positions(index, width, height):
    neighbors = []
    x = index % width
    y = index // width
    # above
    if y > 0:
        if x > 0:
            neighbors.append((y-1) * width + x - 1)
        neighbors.append((y-1) * width + x)
        if x < width - 1:
            neighbors.append((y-1) * width + x + 1)
    # same row
    if x > 0:
        neighbors.append(y * width + x - 1)
    if x < width - 1:
        neighbors.append(y * width + x + 1)
    # below
    if y < height - 1:
        if x > 0:
            neighbors.append((y + 1) * width + x - 1)
        neighbors.append((y + 1) * width + x)
        if x < width - 1:
            neighbors.append((y + 1) * width + x + 1)
    return neighbors


def run_test_task1(tu, test_name):
    test = test_data[test_name]
    expected = test["out"].replace('\r\n', '\n').replace('\r', '\n')
    out = run_raycast(tu, test)
    if not out:
        return tu.FAILURE(f'No output.')
    actual = out.replace('\r\n', '\n').replace('\r', '\n')
    if not actual.endswith('\n'):
        actual += '\n'
    if expected == actual:
        return tu.SUCCESS()
    return tu.FAILURE(f'Incorrect output.\nExpected:\n{expected}\nActual:\n{actual}')



def run_test_task2(tu, test_name):
    test = test_data[test_name]
    error, actual_pixels = get_pixels(tu, test)
    if error is not None:
        return error
    expected_pixels = get_reference_pixels(tu, test)
    if actual_pixels == expected_pixels:
        return tu.SUCCESS()
    return tu.FAILURE('Pixels do not match.')


def run_test_task3(tu, test_name):
    # Test is passed iff all pixes match the expected pixel or at least a neighbor.
    test = test_data[test_name]
    error, actual_pixels = get_pixels(tu, test)
    if error is not None:
        return error
    expected_pixels = get_reference_pixels(tu, test)
    if actual_pixels == expected_pixels:
        return tu.SUCCESS()
    differences = [i for i in range(len(expected_pixels)) if actual_pixels[i] != expected_pixels[i]]
    width = test["width"]
    height = test["height"]
    for i in differences:
        actual = actual_pixels[i]
        if actual not in [expected_pixels[j] for j in get_neighboring_positions(i, width, height)]:
            return tu.FAILURE(f'Pixels do not match.')
    print(f"WARNING: {len(differences)} pixels differ. Test only passed because at least one neighbor matches.")
    return tu.SUCCESS()


def run_test_task4(tu, test_name):
    # We count how many pixels do not match the expected pixel or one if its neighbors.
    # If that error count is higher than 10% of the number of non-black pixels, the test is failed.
    test = test_data[test_name]
    error, actual_pixels = get_pixels(tu, test)
    if error is not None:
        return error
    expected_pixels = get_reference_pixels(tu, test)
    if actual_pixels == expected_pixels:
        return tu.SUCCESS()
    num_black_pixels = expected_pixels.count((0,0,0))
    num_non_black_pixels = len(expected_pixels) - num_black_pixels
    differences = [i for i in range(len(expected_pixels)) if actual_pixels[i] != expected_pixels[i]]
    width = test["width"]
    height = test["height"]
    num_errors = 0
    for i in differences:
        actual = actual_pixels[i]
        if actual not in [expected_pixels[j] for j in get_neighboring_positions(i, width, height)]:
            num_errors += 1
    if num_errors / max(num_non_black_pixels, 1) > 0.1:
        return tu.FAILURE(f'Too many pixels do not match.')
    print(f"WARNING: {len(differences)} pixels differ." if len(differences) > 1 else "WARNING: 1 pixel differs.")
    return tu.SUCCESS()


def run_test_task5(tu, test_name):
    return run_test_task4(tu, test_name)


all_tests = {}
for name, t in test_data.items():
    match t["task"]:
        case 1:
            all_tests[name] = run_test_task1
        case 2:
            all_tests[name] = run_test_task2
        case 3:
            all_tests[name] = run_test_task3
        case 4:
            all_tests[name] = run_test_task4
        case 5:
            all_tests[name] = run_test_task4


all_tests['public.style.cpplint'] = style_test_cpplint
all_tests['public.style.splint'] = style_test
all_tests['public.style.complexity'] = style_complexity

check_secret = False
timeout_secs = 1



