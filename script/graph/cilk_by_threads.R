# Fixed Size
# Args: File Name, Desired Length, Out File
args <- commandArgs(trailingOnly=TRUE)
rawdata <- read.csv(args[1], header = FALSE, sep=";")
names(rawdata) <- c('length', 'threads', 'chunk', 'time')
full_data <- subset(rawdata, length == args[2], select=c(length, threads, chunk, time))
jpeg(args[3])

vals <- unique(full_data$threads)

plot(data$chunk, data$time, type="o", main=paste(args[1], ",\nSize:",args[2], sep=""), xlab="Chunk", ylab="Time")
for (i in 1:length(vals)) {
	data <- subset(full_data, threads == vals[i])
	lines(data$chunk, data$time, col=i)
}
legend(1, length(vals), cex=0.8, c(vals), col=c(1:length(vals)))

axis(1, at=1:length(data$chunk), labels=c(data$chunk))
dev.off()
