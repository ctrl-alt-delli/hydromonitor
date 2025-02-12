<template>
    <v-container fluid class="bg-surface">
      <!-- First Row -->
      <v-row max-width="1200px" class="py-1">
        <!-- First Column -->
        <v-col>
          <v-sheet class="pa-2" height="250">
            <p class="pink-text">Enter date range for Analysis</p>
            <v-divider color="#E30B5C"></v-divider>
            <v-text-field
              label="Start date"
              type="date"
              density="compact"
              variant="solo-inverted"
              style="max-width: 300px;"
              class="mr-5"
              flat
              v-model="start"
            ></v-text-field>
            <v-text-field
              label="End date"
              type="date"
              density="compact"
              variant="solo-inverted"
              style="max-width: 300px;"
              flat
              v-model="end"
            ></v-text-field>
            <v-spacer></v-spacer>
            <v-btn @click="updateLineCharts(),updateCards(),updateHistogramCharts(),updateScatterPlots();" class="text-caption" color="#E30B5C" variant="tonal">Analyze</v-btn>
          </v-sheet>
        </v-col>
  
        <!-- Second Column -->
        <v-col cols="3">
          <v-card
            width="250"
            variant="outlined"
            color="primary"
            density="compact"
            rounded="lg"
          >
          <v-card-item class="mb-n5">
  <v-chip-group class="d-flex flex-row justify-center" color="primaryContainer" variant="flat">
    <v-tooltip bottom>
      <template v-slot:activator="{ on, attrs }">
        <v-chip v-bind="attrs" v-on="on" style="background-color: #E18AAA;">
          {{ temperature.min }}
        </v-chip>
      </template>
      <span class="pink-text">Min</span>
    </v-tooltip>
    <v-tooltip top>
      <template v-slot:activator="{ on, attrs }">
        <v-chip v-bind="attrs" v-on="on" style="background-color: #E18AAA;">
          {{ temperature.range }}
        </v-chip>
      </template>
      <span class="pink-text">Range</span>
    </v-tooltip>
    <v-tooltip end>
      <template v-slot:activator="{ on, attrs }">
        <v-chip v-bind="attrs" v-on="on" style="background-color: #E18AAA;">
          {{ temperature.max }}
        </v-chip>
      </template>
      <span class="pink-text">Max</span>
    </v-tooltip>
  </v-chip-group>
</v-card-item>

            <v-card-item class="text-center">
              <span class="text-h1 text-primary font-weight-bold pink-text">{{ temperature.avg }}</span>
            </v-card-item>
          </v-card>
        </v-col>
  
        <!-- Column 2: Humidity Card (similar to Temperature Card) -->
        <v-col cols="3">
          <v-card
            width="250"
            variant="outlined"
            color="primary"
            density="compact"
            rounded="lg"
          >
          <v-card-item class="mb-n5">
  <v-chip-group class="d-flex flex-row justify-center" color="primaryContainer" variant="flat">
    <v-tooltip bottom>
      <template v-slot:activator="{ on, attrs }">
        <v-chip v-bind="attrs" v-on="on" style="background-color: #E18AAA;">
          {{ humidity.min }}
        </v-chip>
      </template>
      <span class="pink-text">Min</span>
    </v-tooltip>
    <v-tooltip top>
      <template v-slot:activator="{ on, attrs }">
        <v-chip v-bind="attrs" v-on="on" style="background-color: #E18AAA;">
          {{ humidity.range }}
        </v-chip>
      </template>
      <span class="pink-text">Range</span>
    </v-tooltip>
    <v-tooltip end>
      <template v-slot:activator="{ on, attrs }">
        <v-chip v-bind="attrs" v-on="on" style="background-color: #E18AAA;">
          {{ humidity.max }}
        </v-chip>
      </template>
      <span class="pink-text">Max</span>
    </v-tooltip>
  </v-chip-group>
</v-card-item>

            <v-card-item class="text-center">
              <span class="text-h1 text-primary font-weight-bold pink-text">{{ humidity.avg }}</span>
            </v-card-item>
          </v-card>
        </v-col>
      </v-row>
  
      <!-- Second Row -->
      <v-row max-width="1200px">
        <!-- First Column -->
        <v-col cols="12">
          <figure class="highcharts-figure">
            <div id="container"></div>
          </figure>
        </v-col>
  
        <!-- Second Column -->
        <v-col cols="12">
          <figure class="highcharts-figure">
            <div id="container0"></div>
          </figure>
        </v-col>
      </v-row>
  
      <!-- Third Row -->
      <v-row max-width="1200px">
        <!-- First Column -->
        <v-col cols="12">
          <figure class="highcharts-figure">
            <div id="container1"></div>
          </figure>
        </v-col>
  
        <!-- Second Column -->
        <v-col cols="12">
          <figure class="highcharts-figure">
            <div id="container2"></div>
          </figure>
        </v-col>
  
        <!-- Third Column -->
        <v-col cols="12">
          <figure class="highcharts-figure">
            <div id="container3"></div>
          </figure>
        </v-col>
      </v-row>
    </v-container>
  </template>
  
  <script setup>
  /** JAVASCRIPT HERE */
  
  // IMPORTS
  import { ref,reactive,watch ,onMounted,onBeforeUnmount,computed } from "vue";  
  import { useRoute ,useRouter } from "vue-router";
  import { useMqttStore } from '@/store/mqttStore'; // Import Mqtt Store 
  import { storeToRefs } from "pinia"; 
  import { useAppStore } from "@/store/appStore";
  
  // Highcharts, Load the exporting module and Initialize exporting module.
  import Highcharts from 'highcharts';
  import more from 'highcharts/highcharts-more';
  import Exporting from 'highcharts/modules/exporting';
  Exporting(Highcharts);
  more(Highcharts);
   
  // VARIABLES
const router      = useRouter();
const route       = useRoute();  
const Mqtt          = useMqttStore(); 
const AppStore = useAppStore();
const { payload, payloadTopic } = storeToRefs(Mqtt); 

const start   = ref("");
const end    = ref("");
const temperature = reactive({"min":0,"max":0,"avg":0,"range":0});
const humidity = reactive({"min":0,"max":0,"avg":0,"range":0});

const tempHiChart = ref(null);
const humidHiChart = ref(null);
const freqHiChart = ref(null);
const scattertempHiChart = ref(null);
const scatterhumidHiChart = ref(null);

const points = ref(10); // Specify the quantity of points to be shown on the live graph simultaneously.
const shift = ref(false);

const CreateCharts = async () => {
// TEMPERATURE CHART
tempHiChart.value = Highcharts.chart('container', {
    chart: { zoomType: 'x' },
    title: { text: 'Temperature and Heat Index Analysis', align: 'left' },
    subtitle: { text: 'The heat index, also known as the "apparent temperature," is a measure that combines'
    +'air temperature and relative humidity to assess how hot it feels to the human body. The'
    + 'relationship between heat index and air temperature is influenced by humidity levels. As humidity increases, the '
    + 'heat index also rises, making the perceived temperature higher than the actual air temperature', align: 'left' },
    yAxis: {
        title: { text: 'Air Temperature & Heat Index' , style:{color:'#000000'}},
        labels: { format: '{value} °C' }
        },

    xAxis: {
        type: 'datetime',
        title: { text: 'Time', style:{color:'#000000'} },
    },

    tooltip: { shared:true,
        xDateFormat: '%Y-%m-%d %H:%M:%S',  // formats the x value as a date/time
          headerFormat: '<b>{point.key}</b><br/>',
          pointFormat: '<span style="color:{series.color}">{series.name}</span>: <b>{point.y} °C</b><br/>'},

    series: [
    {
        name: 'Temperature',
        type: 'spline',
        data: [],
        turboThreshold: 0,
        color: '#F8C8DC'
    },
    {
        name: 'Heat Index',
        type: 'spline',
        data: [],
        turboThreshold: 0,
        color: '#E30B5C'
    } ],
});

// HUMIDITY CHART
humidHiChart.value = Highcharts.chart('container0', {
    chart: { zoomType: 'x' },
    title: { text: 'Humidity Analysis', align: 'left' },
    yAxis: {
        title: { text: 'Humidity Analysis' , style:{color:'#000000'}},
        labels: { format: '{value} %' }
        },

    xAxis: {
        type: 'datetime',
        title: { text: 'Time', style:{color:'#000000'} },
    },

    tooltip: { shared:true,
        xDateFormat: '%Y-%m-%d %H:%M:%S',  // formats the x value as a date/time
          headerFormat: '<b>{point.key}</b><br/>',
          pointFormat: '<span style="color:{series.color}">{series.name}</span>: <b>{point.y} %</b><br/>'},

    series: [
    {
        name: 'Humidity',
        type: 'spline',
        data: [],
        turboThreshold: 0,
        color: '#E30B5C'
    }],
});

// FREQUENCY CHART
freqHiChart.value = Highcharts.chart('container1', {
    chart: {
        type: 'column',
        zoomType: 'x',  // Enable zooming
    },
    title: {
        text: 'Frequency Distribution Analysis', // Chart title
        align: 'left'
    },
    yAxis: {
        title: {
            text: 'Frequency', // Y-axis title
            style: { color: '#000000' }
        },
        labels: {
            format: '{value}' // Display value without units
        }
    },
    
    tooltip: {
        formatter: function () {
        let unit = "";
        
        // Determine the unit based on series name
        if (this.series.name.toLowerCase().includes("humidity")) {
            unit = "%";
        } else if (this.series.name.toLowerCase().includes("temperature") || this.series.name.toLowerCase().includes("heat index")) {
            unit = "°C";
        }

        return `<b>Frequency:</b> ${this.y} Hz <br/>
                <span style="color:${this.series.color}">${this.series.name}</span>: 
                <b>${this.x} ${unit}</b>`;
    }
    },
    series: [
        {
            name: 'Temperature',
            data: [], // Populate data for temperature
            turboThreshold: 0,
            color: '#E30B5C', // Temperature color 
            pointStart: Date.now(), // Start from the current time
            pointInterval: 3600000, // Set interval of 1 hour between data points
            tooltip: {
                pointFormat: '{point.y} °C', // Display temperature value
            }
        },
        {
            name: 'Humidity',
            data: [], // Populate data for humidity
            turboThreshold: 0,
            color: '#E18AAA', // Humidity color 
            pointStart: Date.now(),
            pointInterval: 3600000,
            tooltip: {
                pointFormat: '{point.y}%', // Display humidity percentage
            }
        },
        {
            name: 'Heat Index',
            data: [], // Populate data for heat index
            turboThreshold: 0,
            color: '#FF69B4', // Heat Index color 
            pointStart: Date.now(),
            pointInterval: 3600000,
            tooltip: {
                pointFormat: '{point.y} °C', // Display heat index value
            }
        }
    ],
});

scattertempHiChart.value = Highcharts.chart('container2', {
    chart: {
        type: 'scatter', // Scatter plot type
        zoomType: 'x' // Enable zooming along x-axis
    },
    title: {
        text: 'Temperature & Heat Index Correlation Analysis',
        align: 'left'
    },
    subtitle: {
        text: 'Visualize the relationship between Temperature and Heat Index as well as revealing patterns or trends in the data',
        align: 'left'
    },
    xAxis: {
        title: {
            text: 'Temperature',
            style: { color: '#000000' }
        },
        labels: {
            format: '{value} °C' // Display temperature values with °C
        }
    },
    yAxis: {
        title: {
            text: 'Heat Index',
            style: { color: '#000000' }
        },
        labels: {
            format: '{value} °C' // Display heat index values with °C
        }
    },
    tooltip: {
        pointFormat: 'Temperature: {point.x} °C <br/> Heat Index: {point.y} °C'
    },
    series: [
        {
            name: 'Analysis',
            type: 'scatter',
            data: [], // Data will be dynamically populated
            color: '#E30B5C', // Use a color to differentiate the points
            marker: {
                radius: 4, // Adjust point size
                symbol: 'circle'
            }
        }
    ]
});

scatterhumidHiChart.value = Highcharts.chart('container3', {
    chart: {
        type: 'scatter', // Scatter plot type
        zoomType: 'x' // Enable zooming along x-axis
    },
    title: {
        text: 'Humidity & Heat Index Correlation Analysis',
        align: 'left'
    },
    subtitle: {
        text: 'Visualize the relationship between Humidityand Heat Index as well as revealing patterns or trends in the data',
        align: 'left'
    },
    xAxis: {
        title: {
            text: 'Humidity',
            style: { color: '#000000' }
        },
        labels: {
            format: '{value} %' // Display humidity values in percentage
        }
    },
    yAxis: {
        title: {
            text: 'Heat Index',
            style: { color: '#000000' }
        },
        labels: {
            format: '{value} °C' // Display heat index in °C
        }
    },
    tooltip: {
        pointFormat: 'Humidity: {point.x} % <br/> Heat Index: {point.y} °C'
    },
    series: [
        {
            name: 'Analysis',
            type: 'scatter',
            data: [], // Data will be dynamically populated
            color: '#E30B5C', // Use a color to differentiate the points
            marker: {
                radius: 4, // Adjust point size
                symbol: 'circle'
            }
        }
    ]
});


};

const updateLineCharts = async () => {
  if (!!start.value && !!end.value) {
    // Convert output from Textfield components to 10 digit timestamps
    let startDate = new Date(start.value).getTime() / 1000;
    let endDate = new Date(end.value).getTime() / 1000;

    // Fetch data from backend
    const data = await AppStore.getAllInRange(startDate, endDate);
    // Create arrays for each plot
    let temperature = [];
    let heatindex = [];
    let humidity = [];

    // Iterate through data variable and transform object to format recognized by highcharts
    data.forEach(row => {
      temperature.push({
        "x": row.timestamp * 1000,
        "y": parseFloat(row.temperature.toFixed(2))
      });
      const heatIndex = row["heat index"];
      heatindex.push({
        "x": row.timestamp * 1000,
        "y": parseFloat(heatIndex.toFixed(2)),
      });
      humidity.push({
        "x": row.timestamp * 1000,
        "y": parseFloat(row.humidity.toFixed(2))
      });
    });

    // Add data to Temperature and Heat Index chart
    tempHiChart.value.series[0].setData(temperature);
    tempHiChart.value.series[1].setData(heatindex);
    humidHiChart.value.series[0].setData(humidity);
  }
};

const updateHistogramCharts = async () => {
    // Retrieve Min, Max, Avg, Spread/Range for Column graph
    if (!!start.value && !!end.value) {
        // 1. Convert start and end dates collected fron TextFields to 10 digit timestamps
        let startDate = new Date(start.value).getTime() / 1000;
        let endDate = new Date(end.value).getTime() / 1000;
        // Subsequently, create startDate and endDate variables and then save the respective timestamps in these
        // variables
        // 2. Fetch data(temp, humid and hi) from backend by calling the getFreqDistro API functions for each
        const temp = await AppStore.getFreqDistro("temperature", startDate, endDate);
        const humid = await AppStore.getFreqDistro("humidity", startDate, endDate);
        const hi = await AppStore.getFreqDistro("heat index", startDate, endDate);

        // 3. create an empty array for each variable (temperature, humidity and heatindex)
        // see example below
        let temperature = [];
        let humidity = [];
        let heatIndex = [];

        // 4. Iterate through the temp variable, which contains temperature data fetched from the backend
        // transform the data to {"x": x_value,"y": y_value} format and then push it to the temperature array created
        // previously
        // see example below
        temp.forEach(row => {
            temperature.push({ "x": row["_id"], "y": row["count"] });
        });

        // 5. Iterate through the humid variable, which contains humidity data fetched from the backend
        // transform the data to {"x": x_value,"y": y_value} format and then push it to the humidity array created
        // previously
        humid.forEach(row => {
            humidity.push({ "x": row["_id"], "y": row["count"] });
        });

        // 6. Iterate through the humid variable, which contains heat index data fetched from the backend
        // transform the data to {"x": x_value,"y": y_value} format and then push it to the heatindex array created
        // previously
        hi.forEach(row => {
            heatIndex.push({ "x": row["_id"], "y": row["count"] });
        });

        // 7. update series[0] for the histogram/Column chart with temperature data
        // see example below
        freqHiChart.value.series[0].setData(temperature);

        // 8. update series[1] for the histogram/Column chart with humidity data
        freqHiChart.value.series[1].setData(humidity);
        // 9. update series[2] for the histogram/Column chart with heat index data
        freqHiChart.value.series[2].setData(heatIndex);
    }
};


const updateCards = async () => {
    // Retrieve Min, Max, Avg, Spread/Range
    if (!!start.value && !!end.value) {
        // 1. Convert start and end dates collected from TextFields to 10-digit timestamps
        let startDate = new Date(start.value).getTime() / 1000;
        let endDate = new Date(end.value).getTime() / 1000;

        // 2. Fetch data from backend by calling the API functions
        const temp = await AppStore.getTemperatureMMAR(startDate, endDate);
        const humid = await AppStore.getHumidityMMAR(startDate, endDate);

        // 3. Complete for min, avg, and range
        temperature.max = temp[0].max.toFixed(1);
        temperature.avg = temp[0].avg.toFixed(1);
        temperature.min = temp[0].min.toFixed(1);
        temperature.range = temp[0].range.toFixed(1);


        // 4. Complete max, min, avg, and range for the humidity variable
        humidity.max = humid[0].max.toFixed(1);
        humidity.avg = humid[0].avg.toFixed(1);
        humidity.min = humid[0].min.toFixed(1);
        humidity.range = humid[0].range.toFixed(1);
    }
};

const updateScatterPlots = async ()=>{
    if(!!start.value && !!end.value){
        // Convert output from Textfield components to 10 digit timestamps
        let startDate = new Date(start.value).getTime() / 1000;
        let endDate = new Date(end.value).getTime() / 1000;
        // Fetch data from backend
        const data = await AppStore.getAllInRange(startDate,endDate);
        
        
        // Create arrays for each plot
        let temperature = [];
        let humidity = [];

        data.forEach(row => {
            const heatIndex = row["heat index"];
            temperature.push([parseFloat(row.temperature.toFixed(2)),parseFloat(heatIndex.toFixed(2))])
            humidity.push([parseFloat(row.humidity.toFixed(2)),parseFloat(heatIndex.toFixed(2))])
            console.log("Temp: ",temperature)
            console.log("Humid: ",humidity)
        });
        
        // Add data to Temperature and Heat Index chart
        scattertempHiChart.value.series[0].setData(temperature);
        scatterhumidHiChart.value.series[0].setData(humidity);
    }
}



// FUNCTIONS
onMounted(()=>{
    // THIS FUNCTION IS CALLED AFTER THIS COMPONENT HAS BEEN MOUNTED
    CreateCharts();
    Mqtt.connect(); // Connect to Broker located on the backend 

    setTimeout( ()=>{ 
        // Subscribe to each topic 
        Mqtt.subscribe("620157584"); 
    },3000); 
});


onBeforeUnmount(()=>{
    // THIS FUNCTION IS CALLED RIGHT BEFORE THIS COMPONENT IS UNMOUNTED
    // unsubscribe from all topics 
    Mqtt.unsubcribeAll(); 
});


</script>
  
  <style scoped>
  .pink-text {
    color: #E30B5C !important;
  }
  .highcharts-figure {
    margin: 0;
    padding: 0;
  }
  Figure {
border: 2px solid black;
}
  </style>
  