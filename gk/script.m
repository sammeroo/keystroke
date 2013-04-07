# Octave script to simulate BPN
# Constants
N = 7;					# Number of input features
alpha = 0.5;				# BPN Training Rate
MAX_ITER = 10000;			# Maximum no. of iterations

filename = "tmp/new_rule_devfile"
file = fopen(filename, "r");

gflag=0;

# Define linecount function:
function retval = linecount (fid)
	count = 0;
	while(fgets(fid) != -1)
		count++;
	endwhile
	retval = count;
endfunction


#[files, err, msg] = readdir("./log/")
#files
#t=1;
#while(t<size(files)(1))
#fn=files(t,1)
#f=fopen(files(t,1),"r")
#t+=1;
#endwhile

#if(index(files(t,1),"log")==0)
#index(files(t,1),".log")
#files(t,1)=0;
#endif
#t+=1
#endwhile
#files






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
W1 = rand(s1, N+1)*(2*exp(1))-(exp(1));		# First layer weights
#W1(1:s1,N+1)=0;
W2 = rand(s2, s1+1)*(2*exp(1))-(exp(1));		# Second layer weights
W3 = rand(1,s2+1)*(2*exp(1))-(exp(1));		# Third layer weights
t_example=zeros(a,N+1);		# Not sure whether a-1 or a
t_example(1:a, 1)=t_example(1:a, 1)+ones(a, 1);
t=zeros(7, a-1);		# Not sure abt this: what it does

#Define Neural Network


function retv=justneural(W3,W2,W1,x,y,m,lrate)
n=1;
v1=W1*x(1:size(x)(1),n);
v1;
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
retv=v3;
endfunction

function [r3,r2,r1] =neural(W3,W2,W1,x,y,m,lrate,no_of_iter,s1,s2,N,gfl)
g3=zeros(size(W3));
g2=zeros(size(W2));
g1=zeros(size(W1));
berror=1000;
pberror=0;
c=0;
flg=0;
vworst=0.2;
iworst=0.8;
iworst-vworst;
v3old=0;
blah2=0;
blah1=0
d=0;
v3avmat=zeros(4,1);
v4avmat=zeros(m-4,1);
tol=.001;
if(gfl==0)
tol=.0001;
endif
while(((berror*berror)>=tol && (d < no_of_iter) ))	# c = no. of iterations
c+=1;
d+=1;
fflush(stdout);
printf("Wait...%d\r", c);

berror=0;
for n=1:m

vworst=0.2;
iworst=0.8;
if(n>=1 && n<2)
n=1;
endif
	v1=W1*x(1:size(x)(1),n);
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


y;
y(1,n);
n;
if(y(1,n)==0)
v3;
v3avmat(n,1)=v3;
n;
flg;
v3old-v3;
endif

if(y(1,n)==1)
v4=v3;
v4;
v4avmat(n-4,1)=v4;
n;
endif



blah1=(sum(v3avmat))/4
blah2=sum(v4avmat)/(m-4)

if(v3>blah1 && y(1,n)==0)
n;
endif



y(1,n);
del4=v3-y(1, n);		# m was n before. Dunno how
berror+=abs(del4);
del3=[W3(1,1);(W3(1,2:size(W3)(2))*del4)'];
del2=(W2(1:size(W2)(1),2:size(W2)(2))'*del3(2:size(del3)(1),1));
#del1=(W1(1:size(W1)(1),2:size(W1)(2))'*del2);
#del2=[(W2(1:size(W2)(1),1))';(W2(1:size(W2)(1),2:size(W2)(2))'*del3(2:size(del3)(1),1))']';

g3(1,2:size(g3)(2))+=(v2*del4)';
g3;

for j=1:(size(del3)(1)-1)
	g2(j,2:size(g2)(2))+=(v1')*del3(j+1,1);	
endfor
g2;

#for j=1:(size(del2)(1)-1)
#	g1(j,2:size(g1)(2))+=(x(2:size(x)(1),n)')*del2(j+1,1);	
#endfor

if((v3>=0.9 && y(1,n)==0) || (v3>=.5 && y(1,n)==0 && c>=500) || (v3<=.25 && y(1,n)==1 && c>=500) || (v3>=.25 && y(1,n)==0 && c>=5000))
W1 = rand(s1, N+1)*(2*exp(1))-(exp(1));		# First layer weights
W2 = rand(s2, s1+1)*(2*exp(1))-(exp(1));		# Second layer weights
W3 = rand(1,s2+1)*(2*exp(1))-(exp(1));
if(gfl==0)
W1(1:s1,N+1)=0;
endif
g3=zeros(size(W3));
g2=zeros(size(W2));
g1=zeros(size(W1));
c=0;

if(c>9000)
no_of_iter+=1000;
endif
flg=0;

endif



endfor

berror/=m;
g3/=m;
g2/=m;
#g1/=m;
if((berror*berror)>=(pberror*pberror) && c>1)
lrate=-lrate;
endif

W3=W3-lrate*g3;
W2=W2-lrate*g2;
#W1=W1-lrate*g1;		# ADDED
pberror=berror;
berror;

endwhile
c
v3avmat
v4avmat
berror/m;
r1=W1;			# ADDED
r2=W2;
r3=W3;

endfunction
#end neural

gradvec1=[];
gradvec2=[];

y = zeros(no_of_training_sets, 1);
z=0;
k=1;
y2=0
for i = 1:no_of_training_sets
	# BPN code goes here
#	y = 1;					# Set output to 1, as these are valid training examples
	x=[fscanf(file, "%f", N)];
        y2=fscanf(file, "%d", 1);
	if(sum(x')>3)
        t_example(k,2:N+1) = x';
        y(k) = y2		# Read training examples one by one
	k+=1;
        endif
        if(sum(x')<3)
	z+=1;
	endif
	# Calculate S3 first	
endfor
no_of_training_sets-=z;
no_of_training_sets
xyz=(2*t_example(4,N+1)-t_example(6,N+1)-t_example(10,N+1))/2;
W1(1:s1,N+1)=0
if(xyz>=-8 && xyz<=8)
gflag=0;
W1(1:s1,N+1)=0
endif
t=t_example'				# Concatenate t_example with 1 for handling bias/threshold
#t_mean=sum(t);
#t_mean/=no_of_training_sets;
#for i=1:size(t)(1)
#t(i,2:size(t)(2))-=t_mean(1,2:size(t)(2));
#endfor
#t=t';
x=1;
#y=ones(1,no_of_training_sets);
#y(1,1:10)=0;
#printf("Training done. Testing now...")
lrate=0.5;
y=y';
y;
#W3=[-2.27768   2.58221  -0.84384  -1.40985   1.75463];
#W1=[0.80368   1.49272  -2.61871   0.98272  -1.92665   1.89280  -0.74042   2.26297;
#  -2.28112  -2.44024   2.68668  -1.41326   0.33211   2.52917  -0.79726  -1.37172;
#  -2.37115   0.50225  -1.30466   0.47093  -1.86840   1.99646  -0.82547  -1.38007;
#   0.78717  -2.38019   2.01310   0.74667   1.41930   2.31115   1.50656  -1.65907;
#   0.42435   0.53333   0.43355   1.54637   1.85866   0.63927  -0.89366  -1.69022;
#  -2.23108  -1.45883   1.53298   1.29328  -1.69817   2.37066   0.24167  -1.32617];
#W2=[-2.107657   1.803260  -1.020146  -0.248187  -1.750668  -0.697149   1.040371;
#  -1.041580   2.470690   0.519825   1.481513   1.245270  -0.218098   0.514059;
#  -1.575078   1.575812   1.293367  -2.147071   1.326889   0.038736  -0.814186;
#   0.657758   1.254462   1.401925   1.401029  -1.597444  -1.606550  -1.719538];
#W3 = ones(1, 5);
#W1 = ones(6, 8);
#W2 = ones(4, 7);

W3i=W3;
W2i=W2;
W1i=W1;
t;
#no_of_training_sets=10;
[W3, W2, W1] =neural(W3,W2,W1,t,y,no_of_training_sets,lrate, MAX_ITER,s1,s2,N,gflag);
W3;
W2;
W1;
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
xyz
#t2 = [7.830769 35.923077 7.492308 40.923077 8.754209 5.536491 48.577273]';		# Value corresponding to 1
#t3=[1 -0.009147 -7.156816 0.259630 23.203220 -0.102958 -0.275683 -7.377763]';			# Values corresponding to 0
#justneural(W3,W2,W1,t2,y,no_of_training_sets,lrate)
#justneural(W3,W2,W1,t3,y,no_of_training_sets,lrate)
