args <- commandArgs(trailingOnly=TRUE)
file1 <- read.csv(args[1], header = FALSE, sep=";")
file2 <- read.csv(args[2], header = FALSE, sep=";")
names(file1) <- c('m', 'n', 'p', 'time')
names(file2) <- c('m', 'n', 'p', 'time')

fFilter1 <- subset(file1, n==args[4] & m==args[3])
fFilter2 <- subset(file2, n==args[4] & m==args[3])

jpeg(args[5])
plot(fFilter1$p, fFilter1$time, type="o", xlab="Processes", ylab="Time", col="blue")
plot(fFilter2$p, fFilter2$time, type="o", col="red")

legend(1, max(fFilter2$time, fFilter1$time), ps, c("Reduce Scatter", "All Gather"), col=c("blue", "red"), lty=1, pch=1)
dev.off()