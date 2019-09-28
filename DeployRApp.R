
currDir <- dirname(rstudioapi::getSourceEditorContext()$path)
setwd(currDir)
library(rsconnect)
rsconnect::deployApp('.')


