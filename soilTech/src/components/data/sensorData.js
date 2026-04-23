const CHANNEL_ID = 3347358;
const READ_API_KEY = "04YHH9CSCBNM0EVX";

export async function getSensorData() {
  try {
    const url = `https://api.thingspeak.com/channels/${CHANNEL_ID}/feeds.json?api_key=${READ_API_KEY}&results=1000`;

    const response = await fetch(url);

    if (!response.ok) {
      throw new Error("Failed to fetch sensor data");
    }

    const data = await response.json();

    const feeds = data.feeds || [];

    const cleaned = feeds.map((item) => ({
      entry_id: item.entry_id,
      created_at: item.created_at,
      nitrogen: item.field1,
      phosphorus: item.field2,
      potassium: item.field3,
      pH: item.field4,
    }));

    return cleaned;
  } catch (error) {
    console.error("Error fetching sensor data:", error);
    return [];
  }
}

