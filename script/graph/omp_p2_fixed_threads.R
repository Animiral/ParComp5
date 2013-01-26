#Output for eqal matrices.
args <- commandArgs(trailingOnly=TRUE)
print(paste("OMP P2 Fixed Size, ", args, sep=""))
rawdata <- read.csv(args[1], header = FALSE, sep=";")
names(rawdata) <- c('m', 'n', 'threads', 'time')
data <- subset(rawdata, threads == args[2], select=c(m, n, threads, time))
jpeg(args[3], width=1024, height=1024)
plot(data$m, data$time, type="n", main=paste("Fixed Threads\n", args[1], ":",args[2], sep=""), xlab="Size", ylab="Time")

n <- unique(data$n)

for (i in 1:length(n)){
	f_data <- subset(data, data$n == n[i])
	lines(f_data$m, f_data$time, col=i)
}

vals <- 1:length(n)
legend(1, 0.025, n, cex=0.8, col=vals, , pch=1, lty=1)
dev.off()
