#!/usr/bin/env python3

from dfa import DFA
import dfa_utils

import os
import subprocess
import sys
import argparse

import re

'''
    Copy cts
'''
def copy_cts(path_src, path_dst):
    # get path to folder of path_src
    dir_path = os.path.dirname(os.path.abspath(path_src))
    # change directory
    os.chdir(dir_path)
    # copy
    p = subprocess.Popen(['cp', path_src, path_dst])
    p.wait()

'''
    Use romeo cli with cts input
'''
def use_romeo_cli(path_to_cts, path_to_output):
    # get path of folder
    dir_path = os.path.dirname(os.path.abspath(path_to_cts))
    # switch current working directory
    os.chdir(dir_path)
    # launch romeo cli and store output
    out = subprocess.check_output([path_romeo, '-v', '-m', path_to_cts]).decode("utf_8")
    # write output to file
    with open(path_to_output, 'w', encoding='UTF-8') as f:
        f.write(str(out))
    
'''
    Write tctl formula to cts file
'''
def write_tctl(path_to_cts, tctl_formula):
    # get path folder
    dir_path = os.path.dirname(os.path.abspath(path_to_cts))
    # switch current working directory
    os.chdir(dir_path)
    # get name of cts file
    name_cts = os.path.basename(os.path.abspath(path_to_cts))
    with open(name_cts, 'r') as read_file:
        lines = read_file.readlines()
    with open(name_cts, 'w') as write_file:
        for line in lines:
            if line.find("check") != -1 or line.find("simulate") != -1:
                write_file.write(tctl_formula)
            else:
                write_file.write(line)

'''
    Write a marking to a cts file
    Element in list_place_restart will be set to 1
    Element in list_place_reset and 'award' will be set to 0
'''
def write_marking_to_cts(path_to_cts, marking_dict, with_event=False):
    with open(path_to_cts, 'r') as read_file:
        lines = read_file.readlines()
    flag = 0
    for line in lines:
        if line.find("initially") != -1:
            start = lines.index(line)
            flag = flag+1
        if line.find("transition") != -1 and flag>0 and flag<2:
            flag = flag+1
            end = lines.index(line)
    new_lines = []
    for line in lines:
        if lines.index(line) == start:
            new_lines.append("initially { \n")
            new_lines.append("uint32_t ")
            dict_key = marking_dict.keys()
            my_var = ""
            for element in dict_key:
                if element == 'award':
                    my_var = my_var+(str(element) + " = " + str(0) + ";\n")
            new_lines.append(my_var)    
            my_var = ""
            new_lines.append("int ")
            for element in dict_key:
                if with_event:
                    if element in list_place_reset:
                        my_var = my_var+(str(element) + " = " + str(0) + ",")
                    elif element in list_place_restart or element in list_event:
                        my_var = my_var+(str(element) + " = " +str(1)+",")
                    elif element == "award":
                        pass
                    else:
                        my_var = my_var+(str(element)+" = "+str(marking_dict[element])+",")
                else:
                    if element in list_place_reset:
                        my_var = my_var+(str(element) + " = " + str(0) + ",")
                    elif element in list_place_restart:
                        my_var = my_var+(str(element) + " = " +str(1)+",")
                    elif element == "award":
                        pass
                    else:
                        my_var = my_var+(str(element)+" = "+str(marking_dict[element])+",")

            my_var = my_var[:-1]+";"+" }\n"
            new_lines.append(my_var)
        elif lines.index(line) > start and lines.index(line)< end:
            pass
        else:
            new_lines.append(line)
    
    new_lines = ''.join(new_lines)
    with open(path_to_cts, 'w', encoding='UTF-8') as write_file:
        write_file.write(new_lines)

'''
    Remove info line from output of romeo cli
'''
def clean_output_romeo_cli(path_to_output):
    # get lines from output
    with open(path_to_output, encoding='UTF-8') as output:
        output_lines = output.readlines()
    # remove info line if any
    output_lines = [line for line in output_lines if line.startswith("[info]") != True]
    # rewrite file without info lines
    with open(path_to_output, 'w') as write_file:
        for line in output_lines:
            write_file.write(line)

'''
    Get result from maxv award
'''
def get_result_from_output(path_to_output):
    with open(path_to_output, 'r', encoding='UTF-8') as result:
        result_lines = result.readlines()
    for line in result_lines:
        if line != '0\n':
            return True, line[:-1]
        break
    return False, 0

'''
    Retrieve marking and cost from simulation of mincost maxv trace
'''
def retrieve_marking_from_output(path_to_output):
    new_dict = dict()
    with open(path_to_output, 'r') as read_file:
        lines = read_file.readlines()

    marking = lines[0]
    cost = lines[1]

    if marking.endswith("\n"):
        marking = marking[:-2]

    marking = marking.split(" ")
    for value in marking:
        value = value.split("=")
        if value[0] == "award":
            award = value[1]
            new_dict[value[0]] = value[1]
        elif value[0] in list_place_reset:
            new_dict[value[0]] = str(0)
        else:
            new_dict[value[0]] = str(value[1])
    cost = cost[7:]
    cost = cost.split(" ")
    return new_dict, cost[0], award

def gen_oil_objects(dfa):
    with open('state_transition.oil', 'w') as write_file:
        for state in dfa.get_states():
            if state == 'Start':
                write_file.write('STATE '+state+' {INITIAL_STATE=TRUE;};\n')
            else:
                write_file.write('STATE '+state+' {};\n')
        
        for transitions in dfa.get_transitions():
            for activity_energy, state_end in dfa.get_transitions()[transitions]:
                write_file.write('TRANSITION '+str(transitions)+'_'+str(state_end)+'{\n')
                with open(str(activity_energy)+'.txt', 'r') as read_file:
                     lines = read_file.readlines()
                for line in lines:
                    write_file.write('  TASK='+line.rstrip()+';\n')
                write_file.write('  FROM_STATE='+str(transitions)+';\n')
                write_file.write('  TO_STATE='+str(state_end)+';\n')
                tmp_energy = activity_energy.split("-",1)[1]
                if tmp_energy == "1400000000":
                    write_file.write('  ENERGY=3300;\n')

                elif tmp_energy == "1050000000":
                    write_file.write('  ENERGY=2950;\n')

                elif tmp_energy == "700000000":
                    write_file.write('  ENERGY=2600;\n')

                else:
                    write_file.write('  ENERGY=2250;\n')

                write_file.write('};\n')

    return

def gen_oil_objects_resurrect(dfa):
    with open('state_transition_resurrect.oil', 'w') as write_file:
        for state in dfa.get_states():
            if state == 'Start':
                write_file.write('RESURRECT_STATE '+state+' {INITIAL_STATE=TRUE;};\n')
            else:
                write_file.write('RESURRECT_STATE '+state+' {};\n')
        
        list_slope = []
        for transitions in dfa.get_transitions():
            for activity_energy, state_end in dfa.get_transitions()[transitions]:
                with open(str(activity_energy)+'.txt', 'r') as read_file:
                     lines = read_file.readlines()
                for line in lines:
                    if line.rstrip() == "dnnmain" and line.rstrip() not in list_slope :
                        write_file.write('RESURRECT_SLOPE '+line.rstrip()+'_slope{ VALUE = 37;};\n')
                        write_file.write('RESURRECT_TIME_SLOPE '+line.rstrip()+'_time_slope{ VALUE = 75000;};\n')
                        list_slope.append(line.rstrip())
                    if line.rstrip() == "convol1D" and line.rstrip() not in list_slope:
                        write_file.write('RESURRECT_SLOPE '+line.rstrip()+'_slope{ VALUE = 40;};\n')
                        write_file.write('RESURRECT_TIME_SLOPE '+line.rstrip()+'_time_slope{ VALUE = 1890000;};\n')
                        list_slope.append(line.rstrip())
                    if line.rstrip() == "convol1H" and line.rstrip() not in list_slope:
                        write_file.write('RESURRECT_SLOPE '+line.rstrip()+'_slope{ VALUE = 43;};\n')
                        write_file.write('RESURRECT_TIME_SLOPE '+line.rstrip()+'_time_slope{ VALUE = 7380000;};\n') 
                        list_slope.append(line.rstrip()) 
                    if line.rstrip() == "convol1W" and line.rstrip() not in list_slope:
                        write_file.write('RESURRECT_SLOPE '+line.rstrip()+'_slope{ VALUE = 43;};\n')
                        write_file.write('RESURRECT_TIME_SLOPE '+line.rstrip()+'_time_slope{ VALUE = 7630000;};\n')
                        list_slope.append(line.rstrip())
                    if line.rstrip() == "relu1" and line.rstrip() not in list_slope:
                        write_file.write('RESURRECT_SLOPE '+line.rstrip()+'_slope{ VALUE = 43;};\n')
                        write_file.write('RESURRECT_TIME_SLOPE '+line.rstrip()+'_time_slope{ VALUE = 283200;};\n')
                        list_slope.append(line.rstrip())
                    if line.rstrip() == "pooling1" and line.rstrip() not in list_slope:
                        write_file.write('RESURRECT_SLOPE '+line.rstrip()+'_slope{ VALUE = 40;};\n')
                        write_file.write('RESURRECT_TIME_SLOPE '+line.rstrip()+'_time_slope{ VALUE = 1948000;};\n')
                        list_slope.append(line.rstrip())
                    if line.rstrip() == "convol2" and line.rstrip() not in list_slope:
                        write_file.write('RESURRECT_SLOPE '+line.rstrip()+'_slope{ VALUE = 42;};\n')
                        write_file.write('RESURRECT_TIME_SLOPE '+line.rstrip()+'_time_slope{ VALUE = 10290000;};\n')
                        list_slope.append(line.rstrip())
                    if line.rstrip() == "relu2" and line.rstrip() not in list_slope:
                        write_file.write('RESURRECT_SLOPE '+line.rstrip()+'_slope{ VALUE = 44;};\n')
                        write_file.write('RESURRECT_TIME_SLOPE '+line.rstrip()+'_time_slope{ VALUE = 162930;};\n')
                        list_slope.append(line.rstrip())
                    if line.rstrip() == "pooling2" and line.rstrip() not in list_slope:
                        write_file.write('RESURRECT_SLOPE '+line.rstrip()+'_slope{ VALUE = 39;};\n')
                        write_file.write('RESURRECT_TIME_SLOPE '+line.rstrip()+'_time_slope{ VALUE = 945930;};\n')  
                        list_slope.append(line.rstrip())   
                    if line.rstrip() == "fc1h" and line.rstrip() not in list_slope:
                        write_file.write('RESURRECT_SLOPE '+line.rstrip()+'_slope{ VALUE = 43;};\n')
                        write_file.write('RESURRECT_TIME_SLOPE '+line.rstrip()+'_time_slope{ VALUE = 512640;};\n') 
                        list_slope.append(line.rstrip())              
                    if line.rstrip() == "fc1w" and line.rstrip() not in list_slope:
                        write_file.write('RESURRECT_SLOPE '+line.rstrip()+'_slope{ VALUE = 40;};\n')
                        write_file.write('RESURRECT_TIME_SLOPE '+line.rstrip()+'_time_slope{ VALUE = 305020;};\n')
                        list_slope.append(line.rstrip())
                    if line.rstrip() == "relu3" and line.rstrip() not in list_slope:
                        write_file.write('RESURRECT_SLOPE '+line.rstrip()+'_slope{ VALUE = 44;};\n')
                        write_file.write('RESURRECT_TIME_SLOPE '+line.rstrip()+'_time_slope{ VALUE = 12860;};\n')
                        list_slope.append(line.rstrip())
                    if line.rstrip() == "fc2" and line.rstrip() not in list_slope:
                        write_file.write('RESURRECT_SLOPE '+line.rstrip()+'_slope{ VALUE = 38;};\n')
                        write_file.write('RESURRECT_TIME_SLOPE '+line.rstrip()+'_time_slope{ VALUE = 481130;};\n')
                        list_slope.append(line.rstrip())
                    if line.rstrip() == "prediction" and line.rstrip() not in list_slope:
                        write_file.write('RESURRECT_SLOPE '+line.rstrip()+'_slope{ VALUE = 44;};\n')
                        write_file.write('RESURRECT_TIME_SLOPE '+line.rstrip()+'_time_slope{ VALUE = 540;};\n')
                        list_slope.append(line.rstrip())

            for activity_energy, state_end in dfa.get_transitions()[transitions]:
                write_file.write('RESURRECT_TRANSITION '+str(transitions)+'_'+str(state_end)+'{\n')
                with open(str(activity_energy)+'.txt', 'r') as read_file:
                     lines = read_file.readlines()
                for line in lines:
                    write_file.write('  FUNC='+line.rstrip()+';\n')
                for line in lines:
                    write_file.write('  SLOPE = '+line.rstrip()+'_slope;\n')
                for line in lines:
                    write_file.write('  TIME_SLOPE = '+line.rstrip()+'_time_slope;\n')

                write_file.write('  FROM_STATE='+str(transitions)+';\n')
                write_file.write('  TO_STATE='+str(state_end)+';\n')
                write_file.write('  ENTRY='+str(transitions)+'_'+str(state_end)+'_func;\n')
                tmp_energy = activity_energy.split("-",1)[1]
                if tmp_energy == "1400000000":
                    write_file.write('  ENERGY=3300;\n')

                elif tmp_energy == "1050000000":
                    write_file.write('  ENERGY=2950;\n')

                elif tmp_energy == "700000000":
                    write_file.write('  ENERGY=2600;\n')

                else:
                    write_file.write('  ENERGY=2250;\n')

                write_file.write('};\n')

    return

def remove_transition_same_state(dfa):
    # print(dfa.get_transitions())
    for transitions in dfa.get_transitions():
        output = []
        # print(dfa.get_transitions()[transitions])
        visited = set()
        visited_tuple = set()
        for activity_energy, state_end in dfa.get_transitions()[transitions]:
            if not state_end in visited:
                visited.add(state_end)
                output.append((activity_energy, state_end))
                visited_tuple.add((activity_energy, state_end))
            else:
                for visited_energy, visited_state in visited_tuple :
                    print(activity_energy)
                    print(state_end)
                    print(visited_energy)
                    print(visited_state)
                    if int(activity_energy.split("-",1)[1]) < int(visited_energy.split("-",1)[1]) and visited_state == state_end:
                        visited_tuple.remove((visited_energy, visited_state))
                        visited_tuple.add((activity_energy, state_end))
                        # remove from output
                        output.remove((visited_energy, visited_state))
                        output.append((activity_energy, state_end))
            # print("output")
            # print(output)
            # print("visited : ")
            # print(visited)
        for activity_energy, state_end in dfa.get_transitions()[transitions]:
            if (activity_energy, state_end) not in output:
                dfa.remove_transition(transitions, state_end, activity_energy)
    # print(dfa.get_transitions())
    return

if __name__ == "__main__":

    '''
        Parse argument (cts file)
    '''
    parser = argparse.ArgumentParser(description='Sequence generator')
    parser.add_argument('-f', dest='filename', default=None)

    (args) = parser.parse_args()
    if args.filename is None:
        sys.exit("Usage: -f cts_file")

    path_romeo = "/Applications/Romeo.app/Contents/Resources/Scripts/bin/romeo-cli"
    # cts_name = "/romeo_model/jdeds_sec5"
    # cts_name = "/romeo_model/noName"
    cts = os.path.abspath(args.filename)
    cts_name, extension = os.path.splitext(cts)
    path_cts = cts_name+".cts"
    path_cts_copy = cts_name+"_copy.cts"
    # path_output = os.getcwd()+"/romeo_model/output.txt"
    dir_path = os.path.dirname(os.path.abspath(cts))
    path_output = dir_path+"/output.txt"
    # print(path_output)
    path_graph = cts_name+"graph"

    reward_dict = dict()

    list_reward_dict = []

    '''
        List of energy that will be tested
    '''
    # 3,3V --> 1,9V = 1,4V 
    # 100% : 1,4V
    # 75% : 1.05V
    # 50% : 0.7V
    # 25% : 0.35V
    # 20% : 0.28V
    # 10% : 0.14V
    # in nV
    list_energy = [1400000000, 1050000000, 700000000, 350000000]
    # list_energy = [350000000, 280000000, 140000000]
    # list_energy = [3500000]

    '''
        List of place to reset / restart
    '''
    # list_place_reset = ['P4', 'P5']
    # list_place_restart = ['P1', 'P2', 'P3']
    # list_place_reset = ['ModeAcquisition', 'ModeFFT', 'ModeBufferFFT', 'ModeSend', 'ModeFRAM', 'ReadyMicro', 'ReadyFFT', 'ReadySend']
    list_place_reset = []
    list_place_restart = []
    list_event = []
    # list_place_restart = ['ActivateAudio']
    # list_event = ['EventClassification']
    '''
        Create DFA 
    '''
    # Initialize
    dfa = DFA("")
    dfa.add_state("Start")
    dfa.init = "Start"
    current_state = "Start"

    '''
        Copy CTS file and work with copy
    '''
    copy_cts(path_cts, path_cts_copy)

    '''
        We extract information from cts file and feed it to a dict
    '''
    marking_dict = dict()

    with open(path_cts_copy, 'r') as read_file:
        lines = read_file.readlines()
    lines = [line for line in lines if line.startswith("//") != True]
    lines = [line for line in lines if line.startswith("\n") != True]

    # get start and end point in cts file to replace variable and marking
    flag = 0
    var = ""
    for line in lines:
        if line.find("initially") != -1:
            start = lines.index(line)
            flag = flag + 1
        if line.find("transition") != -1 and flag >0 and flag <2:
            flag = flag + 1
            end = lines.index(line)
    for line in lines:
        if lines.index(line)<end and lines.index(line)>start:
            var = var + line

    var = var.replace('}', '')
    var = var.replace('\n', '')
    var = var.replace(' ', '')
    var = re.split(',|;|\n',var)
    for value in var[:-1]:
        if value.startswith(' ') == True:
            value = value[1:]
        if value.startswith('uint32_t') == True:
            value = value[8:]
        if value.startswith('int') == True:
            value = value[3:]
        if value.endswith('}\n') == True:
            value = value[:-4]
        value = value.split('=')
        marking_dict[value[0]] = value[1]

    '''
        Now we create a dict with state:marking_dict
        To map a state (of our DFA) to a marking
    '''

    state_place_dict = {current_state:marking_dict}

    '''
        We add the dict to a list
    '''
    list_var_place_dict_queue = []
    list_dict_seen = []

    list_name_state = []

    for p_state, p_dict in state_place_dict.items():
        list_var_place_dict_queue.append(p_dict)

    '''
        Iteration over the list of marking dict
    '''

    i = 0
    j = 0
    while(list_var_place_dict_queue):
        '''
            Extract DFA state and add it to a list
        '''

        p_state = [k for k, v in state_place_dict.items() if v == list_var_place_dict_queue[0]]
        p_state = p_state[0]
        list_name_state.append(p_state)

        '''
            We write marking and variable to the cts file
        '''

        write_marking_to_cts(path_cts_copy, list_var_place_dict_queue[0])

        '''
            Now we compute the max award with different energy
        '''
        for energy in list_energy:
            tctl_formula = 'check [cost_mode=poly, restrict] maxv award while(cost<'+str(energy)+')'
            write_tctl(path_cts_copy, tctl_formula)
            use_romeo_cli(path_cts_copy, path_output)
            clean_output_romeo_cli(path_output)
            # check is award != 0
            out, val_award = get_result_from_output(path_output)
            
            if out:
                '''
                    Mincost to max award
                '''
                tctl_formula = 'check [cost_mode=poly, restrict] mincost(award == '+str(val_award)+')'
                write_tctl(path_cts_copy, tctl_formula)
                use_romeo_cli(path_cts_copy, path_output)
                clean_output_romeo_cli(path_output)

                '''
                    Now we have a trace to mincost max award, we simulate it to have the marking
                '''
                with open(path_output, 'r') as read_file:
                    lines = read_file.readlines()
                for line in lines:
                    if line.find("Trace") != -1:
                        trace = line[6:]
                        tctl_formula = "simulate [cost_mode=poly] "+trace
                write_tctl(path_cts_copy, tctl_formula)
                use_romeo_cli(path_cts_copy, path_output)
                clean_output_romeo_cli(path_output)
                
                '''
                    We extract marking from simulation and update state in graph
                '''
                new_dict, cost, award = retrieve_marking_from_output(path_output)

                list_dict_seen = state_place_dict.values()
                
                if new_dict in list_dict_seen:
                    # Add trace in alphabet
                    trace = trace.split(",")
                    dfa_transition = ""
                    flag = 0
                    for symbol in trace:
                        if symbol.startswith(" "):
                            symbol = symbol[1:]
                        if symbol.endswith("\n"):
                            symbol = symbol[:-1]
                        if flag == 0:
                            dfa_transition = symbol
                            flag = 1
                        else:
                            dfa_transition = dfa_transition + "-" + symbol
                    for p_id, p_info in state_place_dict.items():
                        if new_dict == p_info:
                            old_state = p_id

                    with open('a'+str(j)+'-'+str(energy)+'.txt', 'w') as write_file:
                        sequence = dfa_transition.split('-')
                        for item in sequence:
                            if item.startswith("choose") == False:
                                write_file.write(item+'\n') 
                    # dfa_transition = dfa_transition + "_" + str(energy)
                    dfa.add_symbol('a'+str(j)+'-'+str(energy))
                    dfa.add_transition(p_state, 'a'+str(j)+'-'+str(energy), old_state)

                    reward_dict["place_start"] = p_state
                    reward_dict["place_end"] = old_state
                    reward_dict["transition"] = 'a'+str(j)+'-'+str(energy)
                    reward_dict["cost"] = cost
                    reward_dict["award"] = award
                    reward_dict["energy_start"] = energy
                    list_reward_dict.append(reward_dict.copy())
                    j += 1

                else:
                    # We create a state
                    new_state = 'q_'+str(i)
                    i = i + 1
                    state_place_dict[new_state] = new_dict
                    list_var_place_dict_queue.append(new_dict)

                    dfa.add_state(new_state)
                    trace = trace.split(",")
                    dfa_transition = ""
                    flag = 0
                    
                    for symbol in trace:
                        if symbol.startswith(" "):
                            symbol = symbol[1:]
                        if symbol.endswith("\n"):
                            symbol = symbol[:-1]
                        if flag == 0:
                            dfa_transition = symbol
                            flag = 1
                        else:
                            dfa_transition = dfa_transition + '-' + symbol
                    with open('a'+str(j)+'-'+str(energy)+'.txt', 'w') as write_file:
                        sequence = dfa_transition.split('-')
                        for item in sequence:
                            if item.startswith("choose") == False:
                                write_file.write(item+'\n') 
                    # dfa_transition = dfa_transition + '_' + str(energy)
                    dfa.add_symbol('a'+str(j)+'-'+str(energy))
                    dfa.add_transition(p_state, 'a'+str(j)+'-'+str(energy), new_state)
                    reward_dict["place_start"] = p_state
                    reward_dict["place_end"] = new_state
                    reward_dict["transition"] = 'a'+str(j)+'-'+str(energy)
                    reward_dict["cost"] = cost
                    reward_dict["award"] = award
                    reward_dict["energy"] = energy
                    j += 1

                    list_reward_dict.append(reward_dict.copy())
        
            '''
                Now handle event
                Write same marking but with one token in event place
            '''
        # write_marking_to_cts(path_cts_copy, list_var_place_dict_queue[0], with_event=True)

        # for energy in list_energy:

        #     tctl_formula = 'check [cost_mode=poly, restrict] maxv award while(cost<'+str(energy)+')'
        #     write_tctl(path_cts_copy, tctl_formula)
        #     use_romeo_cli(path_cts_copy, path_output)
        #     clean_output_romeo_cli(path_output)
        #     # check is award != 0
        #     out, val_award = get_result_from_output(path_output)

        #     if out:
        #         '''
        #             Mincost to max award
        #         '''
        #         tctl_formula = 'check [cost_mode=poly, restrict] mincost(award == '+str(val_award)+')'
        #         write_tctl(path_cts_copy, tctl_formula)
        #         use_romeo_cli(path_cts_copy, path_output)
        #         clean_output_romeo_cli(path_output)

        #         '''
        #             Now we have a trace to mincost max award, we simulate it to have the marking
        #         '''
        #         with open(path_output, 'r') as read_file:
        #             lines = read_file.readlines()
        #         for line in lines:
        #             if line.find("Trace") != -1:
        #                 trace = line[6:]
        #                 tctl_formula = "simulate [cost_mode=poly] "+trace
        #         write_tctl(path_cts_copy, tctl_formula)
        #         use_romeo_cli(path_cts_copy, path_output)
        #         clean_output_romeo_cli(path_output)

        #         '''
        #             We extract marking from simulation and update state in graph
        #         '''
        #         new_dict, cost, award = retrieve_marking_from_output(path_output)

        #         list_dict_seen = state_place_dict.values()

        #         if new_dict in list_dict_seen:
        #             # Add trace in alphabet
        #             trace = trace.split(",")
        #             dfa_transition = ""
        #             flag = 0
        #             for symbol in trace:
        #                 if symbol.startswith(" "):
        #                     symbol = symbol[1:]
        #                 if symbol.endswith("\n"):
        #                     symbol = symbol[:-1]
        #                 if flag == 0:
        #                     dfa_transition = symbol
        #                     flag = 1
        #                 else:
        #                     dfa_transition = dfa_transition + "_" + symbol
        #             for p_id, p_info in state_place_dict.items():
        #                 if new_dict == p_info:
        #                     old_state = p_id

        #             with open('a'+str(j)+'_'+str(energy)+'.txt', 'w') as write_file:
        #                 sequence = dfa_transition.split('_')
        #                 for item in sequence:
        #                     write_file.write(item+'\n') 

        #             # dfa_transition = dfa_transition + "_" + str(energy) + "_" + "event"
        #             dfa.add_symbol('a'+str(j)+'_'+str(energy))
        #             dfa.add_transition(p_state, 'a'+str(j)+'_'+str(energy), old_state)

        #             reward_dict["place_start"] = p_state
        #             reward_dict["place_end"] = old_state
        #             reward_dict["transition"] = 'a'+str(j)+'_'+str(energy)
        #             reward_dict["cost"] = cost
        #             reward_dict["award"] = award
        #             reward_dict["energy_start"] = energy
        #             list_reward_dict.append(reward_dict.copy())
        #             j += 1

        #         else:
        #             # We create a state
        #             new_state = 'q_'+str(i)
        #             i = i + 1
        #             state_place_dict[new_state] = new_dict
        #             list_var_place_dict_queue.append(new_dict)

        #             dfa.add_state(new_state)
        #             trace = trace.split(",")
        #             dfa_transition = ""
        #             flag = 0

        #             for symbol in trace:
        #                 if symbol.startswith(" "):
        #                     symbol = symbol[1:]
        #                 if symbol.endswith("\n"):
        #                     symbol = symbol[:-1]
        #                 if flag == 0:
        #                     dfa_transition = symbol
        #                     flag = 1
        #                 else:
        #                     dfa_transition = dfa_transition + '_' + symbol
        #             with open('a'+str(j)+'_'+str(energy)+'.txt', 'w') as write_file:
        #                 sequence = dfa_transition.split('_')
        #                 for item in sequence:
        #                     write_file.write(item+'\n') 
        #             # dfa_transition = dfa_transition + '_' + str(energy) + '_' + 'event'
        #             dfa.add_symbol('a'+str(j)+'_'+str(energy))
        #             dfa.add_transition(p_state, 'a'+str(j)+'_'+str(energy), new_state)
        #             reward_dict["place_start"] = p_state
        #             reward_dict["place_end"] = new_state
        #             reward_dict["transition"] = 'a'+str(j)+'_'+str(energy)
        #             reward_dict["cost"] = cost
        #             reward_dict["award"] = award
        #             reward_dict["energy"] = energy
        #             j += 1

        #             list_reward_dict.append(reward_dict.copy())

        list_var_place_dict_queue.pop(0)
    # print(dfa.get_alphabet())
    # print(dfa.get_states())
    # print(dfa.get_transitions())

    remove_transition_same_state(dfa)

    dfa_utils.to_png(dfa, filename=path_graph, name='test')

    gen_oil_objects(dfa)
    gen_oil_objects_resurrect(dfa)

    # dfa_utils.to_png(dfa, filename = "graphSeq", name = "graphSeq")