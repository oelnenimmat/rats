import collections

type_info = collections.namedtuple("type_info", "typename scalar_type dimension")

dimension_names = ['x', 'y', 'z', 'w']

def for_each_dimension(s, dimension):
	result = ""
	for i in range(dimension):
		result += s.format(dimension_names[i])
	return result

def generate_declaration(type_info):
	typename = type_info.typename
	scalar_type = type_info.scalar_type
	dimension = type_info.dimension

	declaration = "struct {}\n".format(typename)
	declaration += "{\n"
	for i in range(dimension):
		declaration += "\t{} {};\n".format(scalar_type, dimension_names[i])

	declaration += "\n"

	# Default constructor
	declaration += "\t{}() : ".format(typename)
	for i in range(dimension):
		declaration += "{}(0)".format(dimension_names[i])
		if i < (dimension - 1):
			declaration += ", "
		else:
			declaration += " {}\n"

	# Other constructor
	declaration += "\t{}(".format(typename)
	for i in range(dimension):
		declaration += "{} {}".format(scalar_type, dimension_names[i])
		if i < (dimension - 1):
			declaration += ", "
		else:
			declaration += ") : "
	for i in range(dimension):
		declaration += "{0}({0})".format(dimension_names[i])
		if i < (dimension - 1):
			declaration += ", "
		else:
			declaration += " {}\n"

	declaration += "};\n"
	return declaration

def unary_negate(info, signature_only = False):
	s = "{0} operator - ({0} v)".format(info.typename)

	if signature_only:
		s += ";"
	else:
		s += "\n{\n"
		s += for_each_dimension("\tv.{0} = -v.{0};\n", info.dimension)
		s += "\treturn v;\n"
		s += "}"

	s += "\n\n";

	return s

def add_assignment(info, signature_only = False):
	s = "{0} & operator += ({0} & a, {0} b)".format(info.typename)

	if signature_only:
		s += ";"
	else:
		s += "\n{\n"
		s += for_each_dimension("\ta.{0} += b.{0};\n", info.dimension)
		s += "\treturn a;\n"
		s += "}"

	s += "\n\n"

	return s

def add(info, signature_only = False):
	s = "{0} operator + ({0} a, {0} b)".format(info.typename)

	if signature_only:
		s += ";\n"
	else:
		s += "\n{\n"
		s += "\ta += b;\n"
		s += "\treturn a;\n"
		s += "}\n"

	s += "\n\n"
	
	return s	

def subtract_assignment(info, signature_only = False):
	s = "{0} & operator -= ({0} & a, {0} b)".format(info.typename)

	if signature_only:
		s += ";"
	else:
		s += "\n{\n"
		s += for_each_dimension("\ta.{0} -= b.{0};\n", info.dimension)
		s += "\treturn a;\n"
		s += "}"

	s += "\n\n"

	return s

def subtract(info, signature_only = False):
	s = "{0} operator - ({0} a, {0} b)".format(info.typename)

	if signature_only:
		s += (";\n")
	else:
		s += ("\n{\n")
		s += ("\ta -= b;\n")
		s += ("\treturn a;\n")
		s += ("}\n")

	s += "\n\n"
	
	return s	

def generate_basic_operators(type_info):
	typename = type_info.typename
	scalar_type = type_info.scalar_type
	dimension = type_info.dimension

	s = unary_negate(type_info)

	s += add_assignment(type_info)
	s += add(type_info)

	s += subtract_assignment(type_info)
	s += subtract(type_info)

	return s
	
float3 = type_info("float3", "float", 3)

print(generate_declaration(float3))
print(generate_basic_operators(float3))
