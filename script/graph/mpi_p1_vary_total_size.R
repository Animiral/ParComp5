args <- commandArgs(trailingOnly=TRUE)
print(paste("mpi_p1_vary_total_size.R, Args ", args, sep=""))
rawdata <- read.csv(args[1], header = FALSE, sep=";")
names(rawdata) <- c('m', 'n', 'r', 'c', 'p', 'dtime')
filtered <- subset(rawdata, p == args[2])

uniqueN <- unique(filtered$n)
uniqueM <- uniqueN

jpeg(args[3], width=1024, height=1024)
plot(filtered$n, filtered$dtime, type="o", xlab="n*m", ylab="Time", col="blue")

for (i in 1:length(uniqueN)) {

	filterN <- subset(filtered, n==uniqueN[i])
    thisN <- uniqueN[i]
	for (j in 1:length(uniqueM)) {
		filterM <- subset(filterN, m==uniqueM[j])
		mnDtimeMean <- mean(filterM$dtime)
#		mnRtimeMean <- mean(filterM$rtime)
        thisM <- uniqueM[j]
		pntD <- c((thisM*thisN), mnDtimeMean)
#		pntR <- c((m*n), mnRtimeMean)
		points(pntD, col="blue")
#		points(pntR, col="red")
	}

}

#legend(1, max(filtered$n)

dev.off()
