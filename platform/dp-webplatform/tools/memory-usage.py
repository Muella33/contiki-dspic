import fileinput

start = False;
prt = False;
interestingSegments = [ '.nbss', '.ndata' ]

segment = None;
address = 0
sz = 0
name = ''

def isHex(x):
	return len(x) > 2 and x[0] == '0' and x[1] == 'x'
def readHex(x):
	return int(x, 0)	# guess radix
	

for line in fileinput.input():
	line = line.rstrip()
	if len(line) == 0: 
		continue
	if 'Linker script and memory map' in line:
		start = True
		continue
	if not start:	
		continue
	if ('LOAD ' in line) or ('END GROUP' in line) or ('END GROUP' in line) or ('OUTPUT(' in line):
		continue	
	
	# types of line
	
	
	#  .section   0xaddress 0xextent
	#  .section   0xaddress   0xsize function
	# 	                      0xsize    variable
	parts = line.split()
		
	if line[0] == '.' or line[0] == '_':
		segment = parts[0]
		prt = segment in interestingSegments
	if not prt:
		continue
		
	
	used = 0;
	if isHex(parts[0]):
		#  size   variable
		offset = readHex(parts[0])
		variable = parts[1]
		varSz = offset - last
		last = offset
		print '                   ' + str(varSz) + '   ' + variable

	elif isHex(parts[1]):
		#  .section   0xaddress 0xextent
		#  .section   0xaddress   0xsize function
		if len(parts) == 3:
			#  .section   0xaddress 0xextent
			pass
		else:
			seg = parts[0];
			address = readHex(parts[1]);
			sz = readHex(parts[2]);
			function = parts[3];
			last = address
			print str(seg) + '    ' + hex(address) + '  size: ' + str(sz) +  '  name ' + str(function)
	else:
		print 'bad line "' + line + '"'
	
	# print str(segment) + '    ' + hex(address) + '  size: ' + hex(sz) +  '  name ' + str(name)
		

