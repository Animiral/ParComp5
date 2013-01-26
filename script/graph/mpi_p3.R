args <- commandArgs(trailingOnly=TRUE)
print(paste("MPI P3, Args: ", args, sep=""))
file1 <- read.csv(args[1], header = FALSE, sep=";")
file2 <- read.csv(args[2], header = FALSE, sep=";")
names(file1) <- c('m', 'n', 'p', 'time')
names(file2) <- c('m', 'n', 'p', 'time')

fFilter1 <- subset(file1, n==args[4] & m==args[3])
fFilter2 <- subset(file2, n==args[4] & m==args[3])

if (length(fFilter1$p) < 1 | length(fFilter2$p) < 1) {
    q()
}

fFilter1 <- fFilter1[with(fFilter1, order(p)), ]
fFilter2 <- fFilter1[with(fFilter2, order(p)), ]

jpeg(args[5], width=1024, height=1024)
plot(fFilter1$p, fFilter1$time, type="o", xlab="Processes", ylab="Time", col="blue")
lines(fFilter2$p, fFilter2$time, type="o", col="red")

#legend(1, max(fFilter2$time, fFilter1$time), ps, c("Reduce Scatter", "All Gather"), col=c("blue", "red"), lty=1, pch=1)
dev.off()
