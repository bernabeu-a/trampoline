# --COPYRIGHT--,BSD
#  Copyright (c) 2019, Texas Instruments Incorporated
#  All rights reserved.
# 
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions
#  are met:
# 
#  *  Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
# 
#  *  Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
# 
#  *  Neither the name of Texas Instruments Incorporated nor the names of
#     its contributors may be used to endorse or promote products derived
#     from this software without specific prior written permission.
# 
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
#  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
#  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
#  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
#  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
#  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
#  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
#  OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
#  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
#  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
#  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# --/COPYRIGHT--

#Python script used to convert MSP430 .txt or intel .hex to C array in .h file
# format:
#   python image2c.py src dest struct
#       src:    Source file in MSP430 .txt 
#       dest:   Destination file in .h format
#       struct: Name of output array

import linecache
import sys
import datetime

curr_time = datetime.datetime.now()
time_str = curr_time.strftime("%Y-%m-%d")
address_array=[]
data_array=[]
data_array2=[]
app_count_array=[]
sizecount=0
app_sizecount=0
addr_count=0
i=0
current_number=0
current_number2=0
print( "\n 430txt2c.pl V1.3     Texas Instruments Inc 2019 \n")

#parameters check
if len(sys.argv)!=4:
	print("Input parameters invalid.")
	print( "This script converts an MSP430 .txt file to C array \n" )
	print( "Example usage: \n" )
	print( "python 430txt2c.py src dest struct\n" )
	print( "Parameters:\n" )
	print( "  src\t\t.MSP430 txt source file\n" )
	print( "  dest\t\t.H file to be generated\n" )
	print( "  struct\tC structure\n" )
	exit()
print("Your input parameters are:")
for parameters in sys.argv[1:4]:
	print('  '+parameters)
file430 = sys.argv[1]
fileH = sys.argv[2]
output_array = sys.argv[3]
with open(file430) as file_object:
	contents = file_object.read()
#	print(contents)
#print("##################################### \n")
#Open the .txt
with open(file430) as file_object:
	lines = file_object.readlines()
		
for line in lines:
	if line[0]=='@':
		addr_count+=1
		line="0x"+line[1:]
		line=line.rstrip()
		address_array.append(''.join(line))
		app_count_array.append(app_sizecount)
#		data_array.pop()
		app_sizecount=0
	else:
		if line[0]!='q':
			line="0x"+line
#			print(isinstance(line, str))
			line=line.replace(' \n','\n')
			line=line.replace(' ',' 0x')
#			print(line)
			sizecount+=line.count('0x')
			app_sizecount+=line.count('0x')
			data_array2=line.split()
#			data_array2.pop()
			data_array+=data_array2
#			print(data_array2)		
#print(data_array)
app_count_array.append(app_sizecount)
app_count_array.pop(0)
#print(app_count_array)
#print(sizecount)
#Write .c file
with open(fileH, 'w+') as file_write:
	file_write.write("//File name: " + fileH + "\n")
	file_write.write("//Generated from " + file430 + "\n")	
	file_write.write("// " + time_str + "\n\n")
	file_write.write("#include <stdint.h>\n\n")
	file_write.write('#define ' + output_array + '_SIZE   ' + str(sizecount)+'\n\n')
	file_write.write('const uint32_t ' + output_array + '_Addr['+str(addr_count)+'] = {\n')
	while current_number<addr_count:
		file_write.write('    ' + str(address_array[current_number]) + ',\n')
		current_number+=1
	file_write.write('};\n\n' )
	file_write.write('const uint32_t ' + output_array + '_Size[' + str(addr_count) + '] = {\n')
	current_number=0
	while current_number<addr_count:
		file_write.write('    ' + str(app_count_array[current_number]) + ',\n')
		current_number+=1	
	file_write.write('};\n\n')	
	current_number=0
	i=0
	while current_number<addr_count:
		file_write.write('const uint8_t ' + output_array + '_'+str(current_number)+'[] = {\n')
		current_number2=0
		while current_number2<app_count_array[current_number]:
			file_write.write(str(data_array[i]) + ',')
			current_number2+=1
			i+=1
			if current_number2%16==0:
				file_write.write('\n')
		file_write.write('};\n\n')
		current_number+=1	
	file_write.write('const uint8_t *' + output_array + '_Ptr[' + str(addr_count) + '] = {\n')
	current_number=0
	while current_number<addr_count:
		file_write.write('    ' + output_array + '_' + str(current_number) + ',\n')
		current_number+=1	
	file_write.write('};\n\n')
				
#print("%%%%%%%%%%%%%%%%%% \n")
#print(address_array)
#print(app_count_array)
print("\n********File "+ fileH +" generated OK! ***********\n")
		

		
