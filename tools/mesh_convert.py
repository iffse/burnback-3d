import sys
import json
import meshio
import numpy

def print_help():
	print('''
Usage: python mesh_convert.py [options] filename
Options:
	-p, --pretty: Using pretty output
	-o, --output: Output file name, defaults to input file name with json extension name
	-h, --help: Show this help
''')

output_name = ''
filename = ''
pretty = False

index = 1
while index < len(sys.argv):
	match sys.argv[index]:
		case '-h' | '--help':
			print_help()
			exit()
		case '-p' | '--pretty':
			pretty = True
			index += 1
		case '-o' | '--output':
			output_name = sys.argv[index + 1]
			index += 2
		case _:
			filename = sys.argv[index]
			index += 1

if filename == '':
	print('No input file specified. Use -h or --help for help')
	exit()

try:
	mesh = meshio.read(filename)
except:
	print('Error reading file')
	exit()

data = {
	'point': [],
	'line': [],
	'triangle': [],
	'tetra': [],
	'quad': [],
	'edge': [],
}

conditions = {
	'boundary': [],
	'recession': [],
	'triangle': [],
}

print('Reading mesh data...')

for entry in mesh.cells_dict:
	if entry not in data:
		continue
	data[entry]= mesh.cells_dict[entry].tolist()


data['point'] = mesh.points.tolist()

print('Reading field conditions...')
boundaries = []
recessions = {}
for field in mesh.field_data:
	condition = field.split()
	if condition[0] in ['inlet', 'outlet', 'symmetry', 'condition']:
		boundaries.append(mesh.field_data[field][0])
	elif condition[0] == 'recession':
		if len(condition) == 1:
			recessions[mesh.field_data[field][0]] = 1
		if len(condition) == 2:
			recessions[mesh.field_data[field][0]] = float(condition[1])
		if len(condition) > 2:
			recessions[mesh.field_data[field][0]] = [float(i) for i in condition[1:]]



	condition_code = mesh.field_data[field][0].item()
	match condition[0]:
		case 'inlet':
			conditions['boundary'].append({
				'tag': condition_code,
				'type': 'inlet',
				'description': condition[3:] if len(condition) > 1 else ''
			})
		case 'outlet':
			conditions['boundary'].append({
				'tag': condition_code,
				'type': 'outlet',
				'description': condition[1:] if len(condition) > 1 else ''
			})
		case 'symmetry':
			conditions['boundary'].append({
				'tag': condition_code,
				'type': 'symmetry',
				'description': condition[3:] if len(condition) > 1 else ''
			})
		case 'condition':
			conditions['boundary'].append({
				'tag': condition_code,
				'description': condition[1:] if len(condition) > 1 else ''
			})

print('Assining conditions to boundaries and recession nodes...')
for cell in mesh.cell_data_dict['gmsh:physical']:
	if cell == 'triangle':
		conditions[cell] = mesh.cell_data_dict['gmsh:physical'][cell].tolist()

if recessions != {}:
	conditions['recession'] = [1] * len(data['point'])
	for condition in conditions['tetra']:
		if condition not in recessions:
			continue
		nodes = list(set([node for triangle in data['tetra'] for node in triangle]))
		for node in nodes:
			conditions['recession'][node] = recessions[condition]

# index corrections (from 1)
print('Correcting indices')
for entry in ['triangle', 'tetra']:
	data[entry] = [[node + 1 for node in entry] for entry in data[entry]]

meshOut = {
	'metaData': {
		'nodes': len(data['point']),
		'triangles': len(data['triangle']),
		# 'tetrahedra': len(data['tetra']),
		# Euler formula for planar graphs
		# 'edges': len(data['point']) + len(data['triangle']) - len(data['tetra']) + loops - 1
		'tetrahedra': len(data['tetra']),
		'version': '0.1'
	},
	'mesh': {
		'nodes': data['point'],
		'triangles': data['triangle'],
		'tetrahedra': data['tetra'],
	},
	'conditions': conditions,
}

if output_name == '':
	output_name = filename[:filename.rfind('.')] + '.json'


with open(output_name, 'w') as file:
	if pretty:
		json.dump(meshOut, file, indent=4)
	else:
		json.dump(meshOut, file)

print('Mesh converted to ' + output_name + ' successfully')

