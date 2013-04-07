N = 7;			# No. of features
alpha = 0.5;		# learning rate

read = fopen("tmp/write", "r");
write = fopen("tmp/read", "w");
weights = fopen("weights", "r");

m = fscanf(weights, "%d", 1);
n = fscanf(weights, "%d", 1);

w1 = [];
for i = 1:m
	w1(i, :) = fscanf(weights, "%f", n);
endfor

w1

m = fscanf(weights, "%d", 1);
n = fscanf(weights, "%d", 1);

w2 = [];
for i = 1:m
	w2(i, :) = fscanf(weights, "%f", n);
endfor
w2

m = fscanf(weights, "%d", 1);
n = fscanf(weights, "%d", 1);

w3 = [];
for i = 1:m
	w3(i, :) = fscanf(weights, "%f", n);
endfor
w3

# Define sigmoid function
function retval = sigmoid(x)
	retval = 1/(1 + exp(-x));
endfunction

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

no_of_training_sets = 2;
t1 = fscanf(read, "%f", N);
t1 = [1; t1];
#t1 = [1 -0.278139 9.178967 0.068657 17.561019 0.588394 -0.318278 -15.846174]';		# Value corresponding to 1
t1
y = 1;
out1 = justneural(w3,w2,w1,t1,y,no_of_training_sets,alpha);
t2 = fscanf(read, "%f", N);
t2 = [1; t2];
#t2=[1 -0.277897 -6.794316 -0.202870 22.265720 -0.131193 -0.556368 -7.270263]';			# Values corresponding to 0
t2
out2 = justneural(w3,w2,w1,t2,y,no_of_training_sets,alpha);
out1;
out2;
fprintf(write, "%f %f", out1, out2);
