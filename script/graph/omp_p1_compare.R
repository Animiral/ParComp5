args <- commandArgs(trailingOnly=TRUE)
print(paste("OMP P1 Compare. Args:, ", args, sep=""))
recursive <- read.csv(args[1], header = FALSE, sep=";")
iterative <- read.csv(args[2], header = FALSE, sep=";")
hillis <- read.csv(args[3], header = FALSE, sep=";")
total <- read.csv(args[4], header = FALSE, sep=";")


names(recursive) <- c('length', 'threads', 'ops', 'ops2', 'time')
names(iterative) <- c('length', 'threads', 'ops', 'ops2', 'time')
names(hillis) <- c('length', 'threads', 'ops', 'ops2', 'time')
names(total) <- c('length', 'threads', 'ops', 'ops2', 'time')

f_rec <- subset(recursive, length == args[5] & threads <= 512)
f_ite <- subset(iterative, length == args[5] & threads <= 512)
f_hil <- subset(hillis, length == args[5] & threads <= 512)
f_tot <- subset(total, length == args[5] & threads <= 512)

if (length(f_rec) < 1) {
	q()
}

f_rec <- f_rec[with(f_rec, order(threads)), ]
f_ite <- f_ite[with(f_ite, order(threads)), ]
f_hil <- f_hil[with(f_hil, order(threads)), ]
f_tot <- f_tot[with(f_tot, order(threads)), ]

jpeg(args[6], width=1024, height=1024)
plot(f_rec$threads, f_rec$time, type="o", col="red", xlab="Threads", ylab="Time")
lines(f_ite$threads, f_ite$time, type="o", col="blue")
lines(f_hil$threads, f_hil$time, type="o", col="green")
lines(f_tot$threads, f_tot$time, type="o", col="yellow")

legend(1, max(recursive$time) , c("Recursive", "Iterative", "Hillis-Steele", "Total Sum"), cex=0.8, col=c("red", "blue", "green", "yellow"), pch=1, lty=1);
title("Comparison")	


dev.off()
