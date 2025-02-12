<template>
    <v-container fluid class="bg-surface justify-center">
      <v-row style="max-width: 1200px;">
        <!-- First Column (9 cols) -->
        <v-col cols="9">
          <figure class="highcharts-figure">
            <div id="container"></div> <!-- Temperature Graph -->
          </figure>
        </v-col>
  
        <!-- Second Column (3 cols) -->
        <v-col cols="3">
            <v-card class="mb-5" max-width="500" style="background-color: #FF69B4">
            <v-card-item>
            <span class="text-h3 text-onPrimaryContainer">{{ temperature }}</span>
          </v-card-item>
          <v-card-subtitle>Temperature</v-card-subtitle>
        </v-card>

        <v-card class="mb-5" max-width="500" style="background-color: #FFD1DC">
            <v-card-item>
            <span class="text-h3 text-onTertiaryContainer">{{ heatindex }}</span>
          </v-card-item>
          <v-card-subtitle>Heat Index (Feels like)</v-card-subtitle>
        </v-card>

        <v-card class="mb-5" max-width="500" style="background-color: #FF69B4">
            <v-card-item>
            <span class="text-h3 text-onSecondaryContainer">{{ humidity }}</span>
          </v-card-item>
          <v-card-subtitle>Humidity</v-card-subtitle>
        </v-card>
        </v-col>
      </v-row>
  
      <v-row justify="start" style="max-width: 1200px;">
        <!-- Single Column (9 cols) -->
        <v-col cols="9">
          <figure class="highcharts-figure">
            <div id="container1"></div> <!-- Humidity Graph -->
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
const points = ref(10); // Specify the quantity of points to be shown on the live graph simultaneously.
const shift = ref(false);
const tempHiChart = ref(null); // Chart object
const humidHiChart = ref(null);

const temperature = computed(()=>{
    if(!!payload.value){
        return `${payload.value.temperature.toFixed(2)} °C`;
    }
});

const heatindex = computed(()=>{
    if(!!payload.value){
        return `${payload.value['heat index'].toFixed(2)} °C`;
    }
});

const humidity = computed(()=>{
    if(!!payload.value){
        return `${payload.value.humidity.toFixed(2)} %`;
    }
});

const CreateCharts = async () => {
// TEMPERATURE CHART
tempHiChart.value = Highcharts.chart('container', {
    chart: { zoomType: 'x' },
    title: { text: 'Air Temperature and Heat Index Analysis', align: 'left' },
    yAxis: {
        title: { text: 'Air Temperature & Heat Index' , style:{color:'#000000'}},
        labels: { format: '{value} °C' }
        },

    xAxis: {
        type: 'datetime',
        title: { text: 'Time', style:{color:'#000000'} },
    },

    tooltip: { shared:true, },

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
humidHiChart.value = Highcharts.chart('container1', {
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

    tooltip: { shared:true, },

    series: [
    {
        name: 'Humidity',
        type: 'spline',
        data: [],
        turboThreshold: 0,
        color: '#E30B5C'
    }],
});

};

watch(payload,(data)=> {
    console.log(data);
    if(points.value > 0){ points.value -- ; }
    else{ shift.value = true; }

tempHiChart.value.series[0].addPoint({y:parseFloat(data.temperature.toFixed(2)) ,x: data.timestamp * 1000 },
true, shift.value);

tempHiChart.value.series[1].addPoint({y:parseFloat(data['heat index'].toFixed(2)) ,x: data.timestamp * 1000 },
true, shift.value);

// Update Humidity chart
humidHiChart.value.series[0].addPoint({ y: parseFloat(data.humidity.toFixed(2)), x: data.timestamp * 1000 },
true, shift.value);

})


const updateLineCharts = async ()=>{
    if(!!start.value && !!end.value){
        // Convert output from Textfield components to 10 digit timestamps
        let startDate = new Date(start.value).getTime() / 1000;
        let endDate = new Date(end.value).getTime() / 1000;

        // Fetch data from backend
        const data = await AppStore.getAllInRange(startDate,endDate);

        // Create arrays for each plot
        let temperature = [];
        let heatindex = [];

        // Iterate through data variable and transform object to format recognized by highcharts
        data.forEach(row => {
        temperature.push({"x": row.timestamp * 1000, "y": parseFloat(row.temperature.toFixed(2)) });
        heatindex.push({ "x": row.timestamp * 1000,"y": parseFloat(row.heatindex.toFixed(2)) });
        });

        // Add data to Temperature and Heat Index chart
        tempChart.value.series[0].setData(temperature);
        tempChart.value.series[1].setData(heatindex);
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
        console.log('Subscribed to topic 620157584');
    },3000); 
});


onBeforeUnmount(()=>{
    // THIS FUNCTION IS CALLED RIGHT BEFORE THIS COMPONENT IS UNMOUNTED
    // unsubscribe from all topics 
    Mqtt.unsubcribeAll(); 
});


</script>


<style scoped>
/** CSS STYLE HERE */
Figure {
border: 2px solid black;
}

</style>