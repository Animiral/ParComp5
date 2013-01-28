#Vary the load on different processes for fixed m, n, p

args <- commandArgs(trailingOnly=TRUE)
print(paste("MPI P1 Fixed M, N, P, Args: ", args, sep=""))
rawdata <- read.csv(args[1], header = FALSE, sep=";")
names(rawdata) <- c('m', 'n', 'r', 'c', 'p', 'dtime')

filtered <- subset(rawdata, m == args[2] & n == args[3] & p == args[4])
#At this point, all that varies are r, c, dtime, rtime
if (length(filtered$r) < 1) {
    q()
}

jpeg(args[5], width=1024, height=1024)
plot(filtered$r, filtered$dtime, type="o", xlab="R", ylab="Time", col="blue", lwd=15)
dev.off()
