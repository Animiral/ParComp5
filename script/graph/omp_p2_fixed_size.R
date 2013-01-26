args <- commandArgs(trailingOnly=TRUE)
print(paste("OMP P2 Fixed Size, ", args, sep=""))
rawdata <- read.csv(args[1], header = FALSE, sep=";")
names(rawdata) <- c('m', 'n', 'threads', 'time')
data <- subset(rawdata, select=c(n, m, threads, time))
jpeg(args[4], width=1024, height=1024)

#plot.new()
plot(data$threads, data$time, type="n", main=paste("By Threads\n", args[1], sep=""), xlab="Threads", ylab="Time")


ns <- unique(data$n)
ms <- unique(data$m)
counter <- 0
points <- vector()

for (i in 1:length(ns)) {
	filterN_data <- subset(data, data$n == ns[i])
	for(j in 1:length(ms)) {
		filterM_data <- subset(filterN_data, filterN_data$m == ms[j])
		counter <- counter+1
		lines(filterM_data$threads, filterM_data$time, col=(counter))
		points <- append(points, paste(i, ", ", j, sep=""))
	}
}
legend(1, max(rawdata$time), points, col=1:counter, pch=1, lty=1) 

#axis(1, at=1:length(data$threads), labels=c(data$threads))
dev.off()
