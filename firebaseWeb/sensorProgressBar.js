// Function to convert sensor data to percentage
function convertToPercentage(sensorValue) {
    const maxSensorValue = 100; // Maximum sensor value (when farthest from the object)
    const minSensorValue = 0;   // Minimum sensor value (when closest to the object)

    // Map the sensor value to a percentage
    const percentage = ((maxSensorValue - sensorValue) / (maxSensorValue - minSensorValue)) * 100;

    // Ensure the percentage is within the valid range (0 to 100)
    return Math.min(Math.max(percentage, 0), 100).toFixed(2); // Round to 2 decimal places
}

export { convertToPercentage };
