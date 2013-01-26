# Fixed Size
# Args: File Name, Desired Length, Out File
args <- commandArgs(trailingOnly=TRUE)
print(paste("CILK Fixed Length, Args:", args, sep=""))
rawdata <- read.csv(args[1], header = FALSE, sep=";")
names(rawdata) <- c('length', 'threads', 'chunk', 'time')
full_data <- subset(rawdata, length == args[2], select=c(length, threads, chunk, time))
jpeg(args[3], width=1024, height=1024)

vals <- unique(full_data$threads)

plot(full_data$chunk, full_data$time, type="n", main=paste(args[1], ",\nSize:",args[2], sep=""), xlab="Chunk", ylab="Time")
for (i in 1:length(vals)) {
	data <- subset(full_data, threads == vals[i])
	lines(data$chunk, data$time, col=i)
}
legend(1, max(full_data$time), cex=0.8, c(vals), col=c(1:length(vals)), pch=1, lty=1)

dev.off()
