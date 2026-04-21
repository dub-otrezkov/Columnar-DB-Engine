import os
import re
import sys

SRC_DIR = r"d:\GitHub\Columnar-DB-Engine\src"

def process_file(path):
    # Determine own header for cpp files
    base = os.path.basename(path)
    is_cpp = path.endswith('.cpp')
    own_header = None
    if is_cpp:
        stem = base[:-4]
        own_header = stem + '.h'

    with open(path, 'r', encoding='utf-8', newline='') as f:
        content = f.read()

    # Detect line endings
    if '\r\n' in content:
        nl = '\r\n'
    else:
        nl = '\n'
    lines = content.split(nl)

    # Find the contiguous include block. We'll locate the first #include,
    # then scan forward collecting consecutive include lines and blank lines.
    # Stop when we hit a non-include, non-blank line, or a preprocessor conditional.
    n = len(lines)
    first_inc = -1
    for i, line in enumerate(lines):
        s = line.strip()
        if s.startswith('#include'):
            first_inc = i
            break
    if first_inc == -1:
        return False  # no includes

    # Scan forward
    last_inc = first_inc
    i = first_inc
    includes = []  # list of (line_idx, raw, kind, path)
    preproc_issue = False
    while i < n:
        s = lines[i].strip()
        if s.startswith('#include'):
            # parse
            m = re.match(r'#include\s*([<"])([^>"]+)[>"]', s)
            if not m:
                preproc_issue = True
                break
            kind = 'quote' if m.group(1) == '"' else 'angle'
            inc_path = m.group(2)
            includes.append((i, lines[i], kind, inc_path))
            last_inc = i
            i += 1
        elif s == '':
            i += 1
        elif s.startswith('#') and not s.startswith('#include'):
            # preprocessor directive inside include block - skip file to be safe
            preproc_issue = True
            break
        elif s.startswith('//') or s.startswith('/*'):
            # comment inside includes - skip file to be safe
            preproc_issue = True
            break
        else:
            break

    if preproc_issue:
        return False
    if not includes:
        return False

    # Determine the end of the include block: last_inc (the last #include line)
    block_start = first_inc
    block_end = last_inc  # inclusive

    # Split into groups
    own_list = []
    project_list = []
    system_list = []
    for (_, raw, kind, inc_path) in includes:
        if is_cpp and own_header is not None and kind == 'quote' and os.path.basename(inc_path) == own_header and not own_list:
            own_list.append((raw, inc_path))
        elif kind == 'quote':
            project_list.append((raw, inc_path))
        else:
            system_list.append((raw, inc_path))

    # If own header wasn't matched by basename, also try exact match to stem
    # Already handled above.

    # Sort project alphabetically by quoted path
    project_list.sort(key=lambda x: x[1])
    system_list.sort(key=lambda x: x[1])

    # Build the new block
    new_block_parts = []
    if own_list:
        new_block_parts.append([own_list[0][0]])
    if project_list:
        new_block_parts.append([p[0] for p in project_list])
    if system_list:
        new_block_parts.append([s[0] for s in system_list])

    new_block_lines = []
    for idx, grp in enumerate(new_block_parts):
        if idx > 0:
            new_block_lines.append('')
        new_block_lines.extend(grp)

    # Original block lines
    original_block_lines = lines[block_start:block_end+1]

    if original_block_lines == new_block_lines:
        return False

    # Replace
    new_lines = lines[:block_start] + new_block_lines + lines[block_end+1:]
    new_content = nl.join(new_lines)

    with open(path, 'w', encoding='utf-8', newline='') as f:
        f.write(new_content)
    return True


def main():
    modified = 0
    scanned = 0
    for root, dirs, files in os.walk(SRC_DIR):
        # Skip build dirs
        parts = root.lower().replace('/', '\\').split('\\')
        if 'build' in parts:
            continue
        for fn in files:
            if fn.endswith('.h') or fn.endswith('.cpp'):
                full = os.path.join(root, fn)
                # Double-check: skip if 'build' is a path component
                rel = os.path.relpath(full, SRC_DIR).lower()
                if rel.startswith('build\\') or '\\build\\' in rel:
                    continue
                scanned += 1
                try:
                    if process_file(full):
                        modified += 1
                        print(f"MOD: {full}")
                except Exception as e:
                    print(f"ERR: {full}: {e}")
    print(f"Scanned: {scanned}, Modified: {modified}")


if __name__ == '__main__':
    main()
