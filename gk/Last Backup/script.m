# Octave script to simulate BPN
# Constants
N = 7;					# Number of input features
alpha = 0.5;				# BPN Training Rate
MAX_ITER = 100;			# Maximum no. of iterations

filename = "tmp/final_dev";		# File to read from training examples from.
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
function retval = sigmoid(x)
	retval = 1/(1 + exp(-x));
endfunction

#Define Cost function
function returnval=cost(W3,W2,W1,y,h,m,lambda)
	w1=sum(sum(W1.*W1));
	w2=sum(sum(W2.*W2));
	w3=sum(sum(W3.*W3));
	x=y*(log(h))'+(1-y)*(log(1-h))';
	x=x+lambda*(w1+w2+w3)/(2*m);
	returnval=x;
endfunction
#End cost function

a = linecount(file);
no_of_training_sets = a;			# No. of training sets = no. of lines in file
printf("%d\n", no_of_training_sets);

frewind(file);
#usr_pass = fscanf(file, "%s %s", 2);		# Read the first line containing password info. Not to be used for deviation file

s1 = 6;			# Size of layer 1
s2 = 4;			# Size of layer 2
s3 = 1;			# Size of layer 3
W1 = rand(s1, N+1)*(2*exp(1))-(exp(1));		# First layer weights initialized to 1
W2 = rand(s2, s1+1)*(2*exp(1))-(exp(1));		# Second layer weights
W3 = rand(1,s2+1)*(2*exp(1))-(exp(1));		# Third layer weights

t_example=zeros(a,N+1);		# Not sure whether a-1 or a
t_example(1:a, 1)=t_example(1:a, 1)+ones(a, 1);
t=zeros(7, a-1);		# Not sure abt this: what it does

#Define Neural Network


function retv=justneural(W3,W2,W1,x,y,m,lrate)
n=1;
v1=W1*x(1:size(x)(1),n);
	for i=1:size(v1)(1)
	for j=1:size(v1)(2)
#		v1(i,j)=v1(i,j)/1000;
		v1(i,j)=sigmoid(v1(i,j));
	endfor
endfor
v1;
v2=[ones(1,size(v1)(2));v1];
v2=W2*v2;
for i=1:size(v2)(1)
	for j=1:size(v2)(2)
#		v2(i,j)=v2(i,j)/1000;
		v2(i,j)=sigmoid(v2(i,j));
	endfor
endfor
v2;
v3=[ones(1,size(v2)(2));v2];
v3=W3*v3;
for i=1:size(v3)(1)
	for j=1:size(v3)(2)
#		v3(i,j)=v3(i,j)/1000;
		v3(i,j)=sigmoid(v3(i,j));
	endfor
endfor
retv=v3;
endfunction


function [r3,r2,r1] =neural(W3,W2,W1,x,y,m,lrate,no_of_iter)
g3=zeros(size(W3));
g2=zeros(size(W2));
g1=zeros(size(W1));
berror=1000;
pberror=0;
c=0;
while((berror*berror)>=.001 && c < no_of_iter)	# c = no. of iterations
c+=1;

fflush(stdout);
printf("Wait...%d\r", c);

berror=0;
for n=1:m

	v1=W1*x(1:size(x)(1),n);
	for i=1:size(v1)(1)
	for j=1:size(v1)(2)
#		v1(i,j)=v1(i,j)/1000;
		v1(i,j)=sigmoid(v1(i,j));
	endfor
endfor
v1;
v2=[ones(1,size(v1)(2));v1];
v2=W2*v2;
for i=1:size(v2)(1)
	for j=1:size(v2)(2)
#		v2(i,j)=v2(i,j)/1000;
		v2(i,j)=sigmoid(v2(i,j));
	endfor
endfor
v2;
v3=[ones(1,size(v2)(2));v2];
v3=W3*v3;
for i=1:size(v3)(1)
	for j=1:size(v3)(2)
#		v3(i,j)=v3(i,j)/1000;
		v3(i,j)=sigmoid(v3(i,j));
	endfor
endfor
v3;
del4=v3-y(1, n);		# m was n before. Dunno how
berror+=abs(del4);
del3=[W3(1,1);(W3(1,2:size(W3)(2))*del4)'];
del2=(W2(1:size(W2)(1),2:size(W2)(2))'*del3(2:size(del3)(1),1));
del1=(W1(1:size(W1)(1),2:size(W1)(2))'*del2);
#del2=[(W2(1:size(W2)(1),1))';(W2(1:size(W2)(1),2:size(W2)(2))'*del3(2:size(del3)(1),1))']';

g3(1,2:size(g3)(2))+=(v2*del4)';
g3;

for j=1:(size(del3)(1)-1)
	g2(j,2:size(g2)(2))+=(v1')*del3(j+1,1);	
endfor
g2;

#for j=1:(size(del3)(1)-1)
#	g1(j,2:size(g1)(2))+=(v1')*del3(j+1,1);	
#endfor

endfor

berror/=m;
g3/=m;
g2/=m;

if((berror*berror)>=(pberror*pberror) && c>1)
lrate=-lrate;
endif

W3=W3-lrate*g3;
W2=W2-lrate*g2;
W1=W1-lrate*g1;		# ADDED
pberror=berror;
berror;

endwhile
c
berror/m;
r1=W1;			# ADDED
r2=W2;
r3=W3;

endfunction
#end neural

gradvec1=[];
gradvec2=[];

y = zeros(no_of_training_sets, 1);

for i = 1:no_of_training_sets
	# BPN code goes here
#	y = 1;					# Set output to 1, as these are valid training examples
	x=[fscanf(file, "%f", N)];
	y(i) = fscanf(file, "%d", 1);
	t_example(i,2:N+1) = x';		# Read training examples one by one
	
	# Calculate S3 first	
endfor

t=t_example';				# Concatenate t_example with 1 for handling bias/threshold
#t_mean=sum(t);
#t_mean/=no_of_training_sets;
#for i=1:size(t)(1)
#t(i,2:size(t)(2))-=t_mean(1,2:size(t)(2));
#endfor
#t=t';
x=1;
#y=zeros(1,no_of_training_sets);
#y(1,no_of_training_sets-12+1:no_of_training_sets)=1;
#printf("Training done. Testing now...")
lrate=.2;
[W3, W2] =neural(W3,W2,W1,t,y',no_of_training_sets,lrate, MAX_ITER);

# Output weights after training to a file
weight_file = fopen("weights", "w");
fprintf(weight_file, "%d %d\n", rows(W1), columns(W1));
for i = 1:rows(W1)
	for j = 1:columns(W1)
		fprintf(weight_file, "%f ", W1(i,j) );
	endfor
	fprintf(weight_file, "\n");
endfor

fprintf(weight_file, "%d %d\n", rows(W2), columns(W2));
for i = 1:rows(W2)
	for j = 1:columns(W2)
		fprintf(weight_file, "%f ", W2(i,j) );
	endfor
	fprintf(weight_file, "\n");
endfor
fprintf(weight_file, "%d %d\n", rows(W3), columns(W3));
for i = 1:rows(W3)
	for j = 1:columns(W3)
		fprintf(weight_file, "%f ", W3(i,j) );
	endfor
	fprintf(weight_file, "\n");
endfor

#t2 = [1 -0.278139 9.178967 0.068657 17.561019 0.588394 -0.318278 -15.846174]';		# Value corresponding to 1
#t3=[1 -0.009147 -7.156816 0.259630 23.203220 -0.102958 -0.275683 -7.377763]';			# Values corresponding to 0
#justneural(W3,W2,W1,t2,y,no_of_training_sets,lrate)
#justneural(W3,W2,W1,t3,y,no_of_training_sets,lrate)
