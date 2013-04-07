# Octave script to simulate BPN

# Constants

N = 6;					# Number of input features
alpha = 0.5;				# BPN Training Rate

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
no_of_training_sets = a - 1;
# printf("%d\n", no_of_training_sets);

frewind(file);
usr_pass = fscanf(file, "%s %s", 2);		# Read the first line containing password info

# BPN Start



#BPN end

s1 = 6;			# Size of layer 1
s2 = 4;			# Size of layer 2
s3 = 1;			# Size of layer 3
W1 = rand(s1, N+1)*(2*exp(1))-(exp(1));		# First layer weights initialized to 1
W2 = rand(s2, s1+1)*(2*exp(1))-(exp(1));		# Second layer weights
W3 = rand(1,s2+1)*(2*exp(1))-(exp(1));		# Third layer weights
t_example=zeros(a-1,N+1);
t_example(1:(a-1),1)=t_example(1:(a-1),1)+ones(a-1,1);
t=zeros(7,a-1);


#Define Neural Network


function retv=justneural(W3,W2,W1,x,y,m,lrate)
n=1;
v1=W1*x(1:size(x)(1),n);
	x(1:size(x)(1),n);
	for i=1:size(v1)(1)
	for j=1:size(v1)(2)
		v1(i,j)=sigmoid(v1(i,j));
	endfor
endfor
v1;
v2=[ones(1,size(v1)(2));v1];
v2=W2*v2;
for i=1:size(v2)(1)
	for j=1:size(v2)(2)
		v2(i,j)=sigmoid(v2(i,j));
	endfor
endfor
v2;
v3=[ones(1,size(v2)(2));v2];
v3=W3*v3;
for i=1:size(v3)(1)
	for j=1:size(v3)(2)
		v3(i,j)=sigmoid(v3(i,j));
	endfor
endfor
v3;
retv=v3;
endfunction


function [r3,r2,r1] =neural(W3,W2,W1,x,y,m,lrate)
g3=zeros(size(W3));
g2=zeros(size(W2));
g1=zeros(size(W1));
berror=1000;
pberror=0;
c=0;
while((berror*berror)>=.001 && c<2000)
c+=1;
berror=0;
for n=1:m

	v1=W1*x(1:size(x)(1),n);
	x(1:size(x)(1),n);
	for i=1:size(v1)(1)
	for j=1:size(v1)(2)
		v1(i,j)=sigmoid(v1(i,j));
	endfor
endfor
v1;
v2=[ones(1,size(v1)(2));v1];
v2=W2*v2;
for i=1:size(v2)(1)
	for j=1:size(v2)(2)
		v2(i,j)=sigmoid(v2(i,j));
	endfor
endfor
v2;
v3=[ones(1,size(v2)(2));v2];
v3=W3*v3;
for i=1:size(v3)(1)
	for j=1:size(v3)(2)
		v3(i,j)=sigmoid(v3(i,j));
	endfor
endfor
v3;
y;
n;
del4=v3-y(1,n);
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

 
for j=1:(size(del2)(1)-1)
	g1(j,2:size(g1)(2))+=(x(2:size(x)(1),n)')*del2(j+1,1);	
endfor
g1;
endfor

berror/=m;
g3/=m;
g2/=m;
g1/=m;
if((berror*berror)>=(pberror*pberror) && c>1)
lrate=-lrate;
endif

W3=W3-lrate*g3;
W2=W2-lrate*g2;
W1;
W1=W1-lrate*g1;
W1;
pberror=berror;
berror;

endwhile
c
berror/m;
r1=W1;
r2=W2;
r3=W3;
endfunction
#end neural

gradvec1=[];
gradvec2=[];

#for i = 1:no_of_training_sets
#	# BPN code goes here
#	y = 1;					# Set output to 1, as these are valid training examples
#	x=[fscanf(file, "%f", N)];	
#	t_example(i,2:7) = t_example(i,2:7)+x';		# Read training examples one by one
#	
#	# Calculate S3 first	
#endfor
t_example=[1 10 10 10 10 10 10; 1 10 10 10 10 10 10; 1 15 18 -2 19 -5 21];

no_of_training_sets=size(t_example)(1);


t=t_example;				# Concatenate t_example with 1 for handling bias/threshold
t_mean=sum(t);
t_mean/=no_of_training_sets;
for i=1:size(t)(1)
t(i,2:size(t)(2))-=t_mean(1,2:size(t)(2));
endfor
t=t';
y=zeros(1,no_of_training_sets);
y(1,no_of_training_sets)=1;
lrate=2;
p=justneural(W3,W2,W1,t,y,no_of_training_sets,lrate)

[W3, W2, W1] =neural(W3,W2,W1,t,y,no_of_training_sets,lrate);


#z=justneural(W3,W2,W1,t,y,no_of_training_sets,lrate);

