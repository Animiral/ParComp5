args <- commandArgs(trailingOnly=TRUE)
print(paste("OMP P1 Compare. Args:, ", args, sep=""))
recursive <- read.csv(args[1], header = FALSE, sep=";")
iterative <- read.csv(args[2], header = FALSE, sep=";")
hillis <- read.csv(args[3], header = FALSE, sep=";")
total <- read.csv(args[4], header = FALSE, sep=";")


names(recursive) <- c('length', 'wuerste', 'ops', 'ops2', 'time')
names(iterative) <- c('length', 'wuerste', 'ops', 'ops2', 'time')
names(hillis) <- c('length', 'wuerste', 'ops', 'ops2', 'time')
names(total) <- c('length', 'wuerste', 'ops', 'ops2', 'time')

f_rec <- subset(recursive, wuerste = 2 & length < 200000)
f_ite <- subset(iterative, wuerste = 2 & length < 200000)
f_hil <- subset(hillis, wuerste = 2 & length < 200000)
f_tot <- subset(total, wuerste = 2 & length < 200000)

if (length(f_rec) < 1) {
	q()
}

f_rec <- f_rec[with(f_rec, order(length)), ]
f_ite <- f_ite[with(f_ite, order(length)), ]
f_hil <- f_hil[with(f_hil, order(length)), ]
f_tot <- f_tot[with(f_tot, order(length)), ]

jpeg(args[6], width=1024, height=1024)
plot(f_rec$length, f_rec$ops, type="o", col="red", xlab="Length", ylab="Performance Counter")
lines(f_ite$length, f_ite$ops, type="o", col="blue")
lines(f_hil$length, f_hil$ops, type="o", col="green")
lines(f_tot$length, f_tot$ops, type="o", col="yellow")
# lines(f_rec$threads, f_rec$ops2, type="o", pch="22", lty="2", col="red")
# lines(f_ite$threads, f_ite$ops2, type="o", pch="22", lty="2", col="blue")
# lines(f_hil$threads, f_hil$ops2, type="o", pch="22", lty="2", col="green")
# lines(f_tot$threads, f_tot$ops2, type="o", pch="22", lty="2", col="yellow")

#legend(1, max(c(recursive$ops, recursive$ops2)) , c("Recursive Ops", "Iterative Ops", "Hillis-Steele Ops", "Total Sum Ops", "Recursive Array Acc", "Iterative Array Acc", "Hillis-Steele Array Acc", "Total Sum Array Acc"), cex=0.8, col=c("red", "blue", "green", "yellow", "red", "blue", "green", "yellow"), pch=1, lty=1);
title("Performance Counter")	


dev.off()
