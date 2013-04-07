# Octave script to simulate BPN

# Constants

N = 7;					# Number of input features
alpha = 0.5;				# BPN Training Rate
gamma = 1;				# Global value of gamma used in sigmoid

filename = "abhinavchdhry.log";		# File to read from training examples from.
file = fopen(filename, "r");

# Define linecount function:
function retval = linecount (fid)
	count = 0;
	while(fgets(fid) != -1)
		count++;
	endwhile
	retval = count;
endfunction

# Define error calculation function
function retval = error()
	
endfunction

# Define sigmoid function
function varargout = sigmoid(varargin)
	gamma = 1;				# Sigmoid scaling factor
	for i = 1:length(varargin)
		varargout{i} = 1/(1 + exp(-gamma * varargin{i}));
	endfor
endfunction

a = linecount(file);
no_of_training_sets = a - 1;
# printf("%d\n", no_of_training_sets);

frewind(file);
t_example = fscanf(file, "%s %s", 2);		# Read the first line containing password info

# BPN Start

s1 = N;			# Size of layer 1, input layer
s2 = 8;			# Size of layer 2
s3 = 7;			# Size of layer 3
s4 = 1;			# Size of output layer/no. of output neurons

W1 = ones(s2, s1+1);		# Weights of first layer/input layer
W2 = ones(s3, s2+1);		# weights of second layer/first hidden layer
W3 = ones(s4, s3+1);		# Weights of third layer/second hidden layer

for i = 1:no_of_training_sets
	# BPN code goes here
	y = 1;					# Set output to 1, as these are valid training examples

	t_example = fscanf(file, "%lf", N);		# Read training examples one by one
	t = [1; t_example];				# Concatenate t_example with 1 for handling bias/threshold
	
	# Forward propagation: output calculation

	O1 = t_example;
	temp1 = W1*t;
	O2 = sigmoid(temp1(:));
	temp2 = W2*[1; O2];
	O3 = sigmoid(temp2(:));
	temp3 = W3*[1; O3];
	O4 = sigmoid(temp3);

	# Error backpropagation
	
	E4 = -(y*log(O4) + (1-y)*log(1-O4));		# Error in output layer
	del4 = E4*gamma*sigmoid(temp3)*sigmoid(temp3);	# Delta for output layer
	W3 = W3 + alpha*del4*[1; O3];			# Weight update for last layer

	
endfor
