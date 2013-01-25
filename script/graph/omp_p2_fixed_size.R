args <- commandArgs(trailingOnly=TRUE)
rawdata <- read.csv(args[1], header = FALSE, sep=";")
names(rawdata) <- c('n', 'm', 'threads', 'time')
data <- subset(rawdata, select=c(n, m, threads, time))
jpeg(args[4])

#plot.new()
plot(data$threads, data$time, type="n", main=paste("By Threads\n", args[1], sep=""), xlab="Threads", ylab="Time")


ns <- unique(data$n)
ms <- unique(data$m)


for (i in 1:length(ns)) {
	filterN_data <- subset(data, data$n == ns[i])
	for(j in 1:length(ms)) {
		filterM_data <- subset(filterN_data, filterN_data$m == ms[i])
		lines(filterM_data$threads, filterM_data$time, col=(i*j))
	}
}


#axis(1, at=1:length(data$threads), labels=c(data$threads))
dev.off()
