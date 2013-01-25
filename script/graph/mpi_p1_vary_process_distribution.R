#Vary the load on different processes for fixed m, n, p

args <- commandArgs(trailingOnly=TRUE)
rawdata <- read.csv(args[1], header = FALSE, sep=";")
names(rawdata) <- c('n', 'm', 'r', 'c', 'p' 'dtime', 'rtime')

filtered <- subset(rawdata, m == args[2] & n == args[3] & p == args[4])
#At this point, all that varies are r, c, dtime, rtime


jpeg(args[5])
plot(filtered$dtime, filtered$r, type="o", xlab="R", ylab="Time", col="blue")
dev.off()