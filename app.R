
library(shiny)
library(gcookbook)
library(ggplot2)

#Forcr non-scientific notation for axes
options(scipen = 999)

ui <- 
  fluidPage(
      fluidRow(
          column(4, 
              wellPanel(
                  titlePanel("Data Traffic Visualizer"),
                  fileInput( "fileInput", "Please upload .dat file"),
                  h4("Current Restrictions:"),
                  h4("Must have 20 ticks for each header")
              )
          ),
          column(4, align = "center", 
                 uiOutput("config")
          ),
          column(4, align = "center", 
              uiOutput("statistics")
          )
      ),
    
      fluidRow(
          column(12, 
              uiOutput("graph")
          )
      ),
    
      plotOutput("Hist")
  )

server <- function(input, output){
  
  # Store the converted info from C executable
  temp <- reactive({
    
    temp = NULL;
    temp <- input$fileInput;
    if (is.null(temp))
      return(NULL)
    
    cmd_name1 <- ""
    cmd_name2 <- ""
    
    # For Windows: Call C++ executable to parse File
    # cmd_name1 <- "./modifyConfig.exe"; cmd_name2 <- "./datParser.exe"
    
    # For Linux: Call .out to parse File
    # cmd_name1 <- "./modifyConfig.out"; cmd_name2 <- "./datParser.out"
    
    if (Sys.info()[1] == "Windows"){
      cmd_name1 <- "./modifyConfig.exe"; cmd_name2 <- "./datParser.exe"
    } else {
      cmd_name1 <- "./modifyConfig.out"; cmd_name2 <- "./datParser.out"
    }
    
    system2("chmod", args = c("777", cmd_name1))
    system2(cmd_name1, args = c(input$log, input$tick, input$size));
    file_name <- temp$datapath
    system2("chmod", args = c("777", cmd_name2))
    system2(cmd_name2, args = c(file_name))
    
    # Extract Info from output File from C++
    temp_path <- "temp.csv"
    temp <- read.table(temp_path, header = TRUE, sep = ",")
    temp_path <- "stats.csv"
    temp <- c(temp, read.table(temp_path, header = TRUE, sep = ","))
    temp
    
  })
  
  # UI for style and range input 
  output$graph <- renderUI({
    if (is.null(temp()))
      return()
    interval <- temp()$Time.Stamp[2]-temp()$Time.Stamp[1]
    tagList( 
        wellPanel(
            radioButtons("graphChoice", "Select Style", 
                choices = c("Plot" = 1,"Line" = 2,"Hist" = 3), 
                selected = 3,
                inline = TRUE
            ),
            sliderInput("range", width = 2000,
                label = "Relative Time Range(ms)",
                value = c(temp()$Time.Stamp[1], temp()$Time.Stamp[length(temp()$Time.Stamp)]), 
                min = temp()$Time.Stamp[1], 
                max = temp()$Time.Stamp[length(temp()$Time.Stamp)], step = interval
            )
        )
    )
  })
  
  output$config <- renderUI({
    tagList( 
      wellPanel(
        h4("Configuration"),
        textInput("log","Header prefix:",value = "log", 
                  placeholder = "Enter the word preceding header (of the form [..])."),
        textInput("tick","Tick prefix:",value = "tick", 
                  placeholder = "Enter the word preceding tick/time stamp."),
        textInput("size","Size prefix:",value = "size",
                  placeholder = "Enter the word preceding send size.")
      )
    )
  })
  
  # UI for statistic info
  output$statistics <- renderUI({
    if (is.null(temp()))
      return()
    tagList( 
      wellPanel(
        h3(textOutput("timeStart")),
        h3(textOutput("timeFinish")), 
        h3(textOutput("stats"))
      )
    )
  })

  # Plot the main graph
  output$Hist <- renderPlot({
    
    if (is.null(temp()))
      return()
    
    # Select range
    startVal <- input$range[1]
    finishVal <- input$range[2]
    temp1 <- 
        do.call(rbind, 
            Map(data.frame, 
                `Time Stamp`=temp()$Time.Stamp, 
                `Send Size Total`=unlist(temp()$Send.Size.Total)
            )
        )
    temp1 <- subset(temp1, (temp1$Time.Stamp >= startVal)& (temp1$Time.Stamp <= finishVal))
    
    # Select style
    graphStyle <- input$graphChoice
    p <- ggplot(temp1, aes(x=Time.Stamp, y=Send.Size.Total, group = 1))  
    p <- p + labs(x = "Time Stamp \\ms") + labs(y = "Total Send Size \\Byte")
    if (graphStyle == 1){
      p <- p + geom_point(size=1, shape=20) #+ scale_y_discrete( breaks = waiver())
    }
    else if (graphStyle == 2){
      p <- p + geom_point(size=1, shape=20) #+ scale_y_discrete( breaks = waiver())
      p <- p + geom_line(linetype="dotted")
    }
    else {
      p <- p + geom_col()
    }
    
    #Output
    p
  })
  
  # Statistic output : Start time
  output$timeStart <- renderText({ 
    if (is.null(temp()))
      return()
    words <- c("Start Time:", toString(temp()$Start))
    paste(words, collapse = " ");
  })
  
  # Statistic output : End time
  output$timeFinish <- renderText({ 
    if (is.null(temp()))
      return()
    words <- c("End Time:",toString(temp()$Finish))
    paste(words, collapse = " ");
  })
  
  # Statistic output : Interval
  output$stats <- renderText({ 
    if (is.null(temp()))
      return()
    interval <- temp()$Time.Stamp[2]-temp()$Time.Stamp[1]
    paste(c("Interval per Observation:",toString(interval),"ms."), collapse = " ");
  })

}

shinyApp(ui = ui, server = server)
