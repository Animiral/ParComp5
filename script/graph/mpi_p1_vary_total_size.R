#Vary the load on different processes for fixed m, n, p

args <- commandArgs(trailingOnly=TRUE)
rawdata <- read.csv(args[1], header = FALSE, sep=";")
names(rawdata) <- c('n', 'm', 'r', 'c', 'p' 'dtime', 'rtime')
filtered <- subset(rawdata, p == args[2])

uniqueN <- unique(filtered$n)
uniqueM <- unique(filtered$m)

jpeg(args[3], width=1024, height=1024)
plot(filtered$n, filtered$dtime, type="o", xlab="n*m", ylab="Time", col="blue")

for (i in 1:length(uniqueN)) {

	filterN <- subset(filtered, n==uniqueN[i])
	for (j in 1:length(uniqueM)) {
		filterM <- subset(filterN, m==uniqueM[j])
		mnDtimeMean <- mean(filterM$dtime)
		mnRtimeMean <- mean(filterM$rtime)
		pntD <- c((m*n), mnDtimeMean)
		pntR <- c((m*n), mnRtimeMean)
		points(pntD, col="blue")
		points(pntR, col="red"))
	}

}

dev.off()