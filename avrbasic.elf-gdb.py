def prompt_hook(cp):
    e = gdb.parse_and_eval("DISPLAY").string()
    o = int(gdb.parse_and_eval("Y")) * 21 + int(gdb.parse_and_eval("X"))
    e = e[:o] + "_" + e[o+1:]
    head = "+{}+".format("-" * 21)
    
    return "\n{}\n|{}|\n|{}|\n|{}|\n|{}|\n|{}|\n|{}|\n|{}|\n|{}|\n{}\n(gdb) ".format(
        head,
        e[21*0:21*1],
        e[21*1:21*2],
        e[21*2:21*3],
        e[21*3:21*4],
        e[21*4:21*5],
        e[21*5:21*6],
        e[21*6:21*7],
        e[21*7:21*8],
        head)

gdb.prompt_hook = prompt_hook
