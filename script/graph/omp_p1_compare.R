args <- commandArgs(trailingOnly=TRUE)
recursive <- read.csv(args[1], header = FALSE, sep=";")
iterative <- read.csv(args[2], header = FALSE, sep=";")
hillis <- read.csv(args[3], header = FALSE, sep=";")
#total <- read.csv(args[4], header = FALSE, sep=";")

names(recursive) <- c('length', 'threads', 'ops', 'time')
names(iterative) <- c('length', 'threads', 'ops', 'time')
names(hillis) <- c('length', 'threads', 'ops', 'time')
#names(total) <- c('length', 'threads', 'ops', 'time')

f_rec <- subset(recursive, length == args[5])
f_ite <- subset(iterative, length == args[5])
f_hil <- subset(hillis, length == args[5])
#f_tot <- subset(recursive, length == args[5])

jpeg(args[6])
plot(f_rec$threads, f_rec$time, type="o", col="red", xlab="Threads", ylab="Time")
lines(f_ite$threads, f_ite$time, type="o", col="blue")
lines(f_hil$threads, f_hil$time, type="o", col="green")
#plot(f_rec$threads, f_rec$time, type="o")

legend(25, max(recursive$time) , c("Recursive", "Iterative", "Hillis-Steele", "Total Sum"), cex=0.8, col=c("red", "blue", "green", "yellow"), pch=21:24, lty=1:4);
title("Comparison")	


dev.off()
