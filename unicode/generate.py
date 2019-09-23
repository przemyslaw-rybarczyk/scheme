#!/usr/bin/env python3

alphabetic = [False] * 0x110000
uppercase = [False] * 0x110000
lowercase = [False] * 0x110000
numeric = [False] * 0x110000
whitespace = [False] * 0x110000
to_lowercase = [None] * 0x110000
to_uppercase = [None] * 0x110000
to_foldcase = [None] * 0x110000
full_uppercase = [None] * 0x110000
full_foldcase = [None] * 0x110000

def code_range(codes):
    if '..' in codes:
        codes = codes.split('..')
        return range(int(codes[0], 16), int(codes[1], 16) + 1)
    else:
        return [int(codes, 16)]

def open_unicode_file(filename):
    with open(filename) as f:
        for line in f:
            if '#' in line:
                line = line.split('#')[0]
            line = line.strip()
            if not line:
                continue
            yield list(map(str.strip, line.split(';')))

for codes, category in open_unicode_file('DerivedGeneralCategory.txt'):
    codes = code_range(codes)
    if category[0] == 'L' or category == 'Nl':
        for code in codes:
            alphabetic[code] = True
    if category == 'Lu':
        for code in codes:
            uppercase[code] = True
    if category == 'Ll':
        for code in codes:
            lowercase[code] = True
    if category == 'Nd':
        for code in codes:
            numeric[code] = True

for codes, prop in open_unicode_file('PropList.txt'):
    codes = code_range(codes)
    if prop == 'Other_Uppercase':
        for code in codes:
            uppercase[code] = True
            alphabetic[code] = True
    elif prop == 'Other_Lowercase':
        for code in codes:
            lowercase[code] = True
            alphabetic[code] = True
    elif prop == 'Other_Alphabetic':
        for code in codes:
            alphabetic[code] = True
    elif prop == 'White_Space':
        for code in codes:
            whitespace[code] = True

for data in open_unicode_file('UnicodeData.txt'):
    code = int(data[0], 16)
    if data[13]:
        lower = int(data[13], 16)
        if lower & 0xFF0000 != code & 0xFF0000:
            print("Assumption failed: lowercase conversion of %04X is not in the same plane" % code)
            raise SystemExit
        to_lowercase[code] = lower & 0xFFFF if lower != code else None
    if data[14]:
        upper = int(data[14], 16)
        if upper & 0xFF0000 != code & 0xFF0000:
            print("Assumption failed: uppercase conversion of %04X is not in the same plane" % code)
            raise SystemExit
        to_uppercase[code] = upper & 0xFFFF if upper != code else None

for code, status, mapping, _ in open_unicode_file('CaseFolding.txt'):
    code = int(code, 16)
    if status == 'C' or status == 'S':
        mapping = int(mapping, 16)
        if mapping & 0xFF0000 != code & 0xFF0000:
            print("Assumption failed: simple case folding of %04X is not in the same plane" % code)
            raise SystemExit
        to_foldcase[code] = mapping & 0xFFFF if mapping != code else None
    elif status == 'F':
        mapping = list(map(lambda m: int(m, 16), mapping.split()))
        if len(mapping) > 3:
            print("Assumption failed: full case folding of %04X is longer than 3 characters" % code)
            raise SystemExit
        for c in mapping:
            if c >= 0x10000:
                print("Assumption failed: full case folding of %04X contains non-BMP character" % code)
                raise SystemExit
        full_foldcase[code] = mapping

for data in open_unicode_file('SpecialCasing.txt'):
    if data[4]:
        continue
    code = int(data[0], 16)
    if len(data[1].split()) > 1 and code != 0x130:
        print("Assumption failed: code point other than U+0130 has lowercase longer than 1 character")
        raise SystemExit
    mapping = list(map(lambda m: int(m, 16), data[3].split()))
    if len(mapping) > 3:
        print("Assumption failed: uppercase of %04X is longer than 3 characters" % code)
        raise SystemExit
    for c in mapping:
        if c >= 0x10000:
            print("Assumption failed: uppercase of %04X contains non-BMP character" % code)
            raise SystemExit
    if len(mapping) > 1:
        full_uppercase[code] = mapping

used_codes = 0x30000

for c in range(0x10FFFF, -1, -1):
    if alphabetic[c] or numeric[c] or whitespace[c] or to_lowercase[c] != None or to_uppercase[c] != None or to_foldcase[c] != None:
        if c >= used_codes:
            print("The largest code point that has to be considered is %04X." % c)
            print("Readjust the limit to account for that.")
            raise SystemExit
        break

def properties(c):
    if lowercase[c]:
        alpha = 1
    elif uppercase[c]:
        alpha = 2
    elif alphabetic[c]:
        alpha = 3
    else:
        alpha = 0
    return 8 * whitespace[c] + 4 * numeric[c] + alpha

row_table_index_step = 0x8000
row_length = 0x20
row_index = []
row_table_index = []
short_row_table = []
long_row_table = []
very_long_row_table = []
short_row_table_length = []
long_row_table_length = []
for a in range(0, used_codes, row_table_index_step):
    row_table_index.append((len(short_row_table), len(long_row_table)))
    for b in range(a, a + row_table_index_step, row_length):
        short_row = [None] * row_length
        long_row = [None] * row_length
        very_long_row = [None] * row_length
        short_row_used = False
        long_row_used = False
        very_long_row_used = False
        for c in range(b, b + row_length):
            short_row[c - b] = properties(c)
            if properties(c) != properties(b):
                short_row_used = True
            long_row[c - b] = (to_uppercase[c], to_lowercase[c], to_foldcase[c])
            if long_row[c - b] != (None, None, None):
                long_row_used = True
            very_long_row[c - b] = (full_uppercase[c], full_foldcase[c])
            if very_long_row[c - b] != (None, None):
                very_long_row_used = True
        if very_long_row_used or long_row_used:
            for c in range(b, b + row_length):
                long_row[c - b] = list(long_row[c - b])
                for i in range(3):
                    if long_row[c - b][i] == None:
                        long_row[c - b][i] = c & 0xFFFF
                long_row[c - b] = tuple(long_row[c - b])
        if very_long_row_used:
            for c in range(b, b + row_length):
                very_long_row[c - b] = list(very_long_row[c - b])
                for i in range(2):
                    if very_long_row[c - b][i] == None:
                        very_long_row[c - b][i] = ((to_uppercase[c] or c, to_foldcase[c] or c)[i], 0, 0)
                    else:
                        very_long_row[c - b][i] = tuple((very_long_row[c - b][i] + [0, 0, 0])[:3])
                very_long_row[c - b] = tuple(very_long_row[c - b])
            row_index.append(2 * used_codes + len(very_long_row_table))
            very_long_row_table.append((short_row, long_row, very_long_row))
        elif long_row_used:
            row_index.append(used_codes + len(long_row_table) - row_table_index[-1][1])
            long_row_table.append((short_row, long_row))
        elif short_row_used:
            row_index.append(len(short_row_table) - row_table_index[-1][0])
            short_row_table.append(short_row)
        else:
            row_index.append(0xF0 | short_row[0])
    short_row_table_length.append(len(short_row_table) - row_table_index[-1][0])
    long_row_table_length.append(len(long_row_table) - row_table_index[-1][1])

table_switch_point = max(short_row_table_length)
table_switch_point_2 = table_switch_point + max(long_row_table_length)
if table_switch_point_2 + len(very_long_row_table) >= 240:
    print("Assumption failed: too many rows in table index block")
    raise SystemExit

for i in range(used_codes // row_length):
    if row_index[i] >= 2 * used_codes:
        row_index[i] = row_index[i] - 2 * used_codes + table_switch_point_2
    elif row_index[i] >= used_codes:
        row_index[i] = row_index[i] - used_codes + table_switch_point

with open('unicode_data.h', 'w') as f:
    f.write('static const uint8_t table_switch_point = ')
    f.write(str(table_switch_point))
    f.write(';\nstatic const uint8_t table_switch_point_2 = ')
    f.write(str(table_switch_point_2))
    f.write(';\n\n')
    f.write('struct Long_row_table_entry {\n    uint8_t short_data[')
    f.write(str(row_length // 2))
    f.write('];\n    uint16_t long_data[')
    f.write(str(row_length))
    f.write('][3];\n};\n\n')
    f.write('struct Very_long_row_table_entry {\n    uint8_t short_data[')
    f.write(str(row_length // 2))
    f.write('];\n    uint16_t long_data[')
    f.write(str(row_length))
    f.write('][3];\n    uint16_t very_long_data[')
    f.write(str(row_length))
    f.write('][2][3];\n};\n\n')
    f.write('static uint8_t short_row_table[')
    f.write(str(len(short_row_table)))
    f.write('][')
    f.write(str(row_length // 2))
    f.write('] = {\n')
    for short_row in short_row_table:
        f.write('    {')
        for i in range(0, row_length, 2):
            f.write('0x%X%X' % (short_row[i], short_row[i + 1]))
            if i != row_length - 2:
                f.write(', ')
        f.write('},\n')
    f.write('};\n\n')
    f.write('static struct Long_row_table_entry long_row_table[')
    f.write(str(len(long_row_table)))
    f.write('] = {\n')
    for short_row, long_row in long_row_table:
        f.write('    {{')
        for i in range(0, row_length, 2):
            f.write('0x%X%X' % (short_row[i], short_row[i + 1]))
            if i != row_length - 2:
                f.write(', ')
        f.write('},\n     {')
        for i in range(row_length):
            f.write('{0x%04X, 0x%04X, 0x%04X}' % long_row[i])
            if i == row_length - 1:
                f.write('}},\n')
            elif i % 4 == 3:
                f.write(',\n      ')
            else:
                f.write(', ')
    f.write('};\n\n')
    f.write('static struct Very_long_row_table_entry very_long_row_table[')
    f.write(str(len(very_long_row_table)))
    f.write('] = {\n')
    for short_row, long_row, very_long_row in very_long_row_table:
        f.write('    {{')
        for i in range(0, row_length, 2):
            f.write('0x%X%X' % (short_row[i], short_row[i + 1]))
            if i != row_length - 2:
                f.write(', ')
        f.write('},\n     {')
        for i in range(row_length):
            f.write('{0x%04X, 0x%04X, 0x%04X}' % long_row[i])
            if i == row_length - 1:
                f.write('},\n')
            elif i % 4 == 3:
                f.write(',\n      ')
            else:
                f.write(', ')
        f.write('     {')
        for i in range(row_length):
            f.write('{{0x%04X, 0x%04X, 0x%04X}, ' % very_long_row[i][0])
            f.write('{0x%04X, 0x%04X, 0x%04X}}' % very_long_row[i][1])
            if i == row_length - 1:
                f.write('}},\n')
            elif i % 2 == 1:
                f.write(',\n      ')
            else:
                f.write(', ')
    f.write('};\n\n')
    f.write('static uint8_t row_index[')
    f.write(str(len(row_index)))
    f.write('] = {\n    ')
    for i in range(len(row_index)):
        f.write('0x%02X' % row_index[i])
        if i == len(row_index) - 1:
            f.write(',\n')
        elif i % 16 == 15:
            f.write(',\n    ')
        else:
            f.write(', ')
    f.write('};\n\n')
    f.write('static struct { uint8_t (*short_row_table_ptr)[')
    f.write(str(row_length // 2))
    f.write(']; struct Long_row_table_entry *long_row_table_ptr; } row_table_index[')
    f.write(str(used_codes // row_table_index_step))
    f.write('] = {\n')
    for short_row_table_index, long_row_table_index in row_table_index:
        f.write('    {&short_row_table[')
        f.write(str(short_row_table_index))
        f.write('], &long_row_table[')
        f.write(str(long_row_table_index))
        f.write(']},\n')
    f.write('};\n')
