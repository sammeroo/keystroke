function plotter(file)
frewind(file);
N = 10;		# No of trials
M = 6;		# X - axis length
#file = fopen("filename", "r");
y1 = fscanf(file, "%f", N);		# "randomness"
y2 = fscanf(file, "%f", N);		# "claustrophobia"
y3 = fscanf(file, "%f", N);		# "backpropagation"
y4 = fscanf(file, "%f", N);		# "numerology"
x = 1:1:N;

# plot(x, y1, "-", "x", ";randomness;", "color", "r", "linewidth", 2, x, y2, ";claustrophobia;", "color", "k", "linewidth", 2, x, y3, ";backpropagation;", "color", "c", "linewidth", 2, x, y4, ";numerology;", "color", "m", "linewidth", 2)

h = plot(x, y1, "-x", "markersize", 5, "color", "r", "linewidth", 2, x, y2, "-x", "markersize", 5, "color", "k", "linewidth", 2, x, y3, "-x", "markersize", 5, "color", "b", "linewidth", 2, x, y4, "-x", "markersize", 5, "color", "m", "linewidth", 2);
hold;
#plot(x, y2, "-x", "markersize", 5, "color", "k", "linewidth", 2);
#plot(x, y3, "-x", "markersize", 5, "color", "b", "linewidth", 2);
#plot(x, y4, "-x", "markersize", 5, "color", "m", "linewidth", 2);
axis([0, M, 0, 60]);
title("GF data plot for user1", "fontsize", 14);
xlabel("Trial number", "fontsize", 13);
ylabel("Grouping Factor value", "fontsize", 13);
text(M-2, 58, "randomness", "color", "r", "fontsize", 13);
text(M-2, 56, "claustrophobia", "color", "k", "fontsize", 13);
text(M-2, 54, "backpropagation", "color", "b", "fontsize", 13);
text(M-2, 52, "numerology", "color", "m", "fontsize", 13);
print -djpg -F:15 "output.jpg";

endfunction
