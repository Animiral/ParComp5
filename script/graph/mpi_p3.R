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
fFilter2 <- fFilter2[with(fFilter2, order(p)), ]

jpeg(args[5], width=1024, height=1024)

xl <- max(fFilter1$p, fFilter2$p)
yl <- max(fFilter1$time, fFilter2$time)
plot(0, xlab="Processes", ylab="Time", xlim=c(0, xl), ylim=c(0, yl))
lines(fFilter1$p, fFilter1$time, type="o", col="blue")
lines(fFilter2$p, fFilter2$time, type="o", col="red")

legend(1, max(fFilter2$time, fFilter1$time), c(args[1], args[2]), col=c("blue", "red"), lty=1, pch=1)
dev.off()
