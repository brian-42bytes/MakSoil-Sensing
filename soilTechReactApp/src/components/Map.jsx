import { MapContainer, TileLayer, Marker, Popup, useMapEvents } from "react-leaflet";
import { useState } from "react";
import "leaflet/dist/leaflet.css";
import L from "leaflet";

// Fix default marker icon issue
delete L.Icon.Default.prototype._getIconUrl;
L.Icon.Default.mergeOptions({
  iconRetinaUrl: "https://unpkg.com/leaflet@1.9.4/dist/images/marker-icon-2x.png",
  iconUrl: "https://unpkg.com/leaflet@1.9.4/dist/images/marker-icon.png",
  shadowUrl: "https://unpkg.com/leaflet@1.9.4/dist/images/marker-shadow.png",
});

// Component to handle clicks
function MapClickHandler({ addPoint }) {
  useMapEvents({
    click(e) {
      const { lat, lng } = e.latlng;
      addPoint({ lat, lng });
    },
  });
  return null;
}
const POINTS = [
  [0.3476, 32.5825], // Kampala (center)
  [0.3136, 32.5811], // Makindye
  [0.3560, 32.6400], // Ntinda
  [0.3000, 32.6000], // Ggaba
  [0.3900, 32.5800], // Kawempe
  [0.2800, 32.5500], // Lubowa
  [0.4200, 32.6200], // Kira
  [0.2500, 32.5300], // Entebbe road area
  [0.5000, 32.4800], // Wakiso
  [0.6000, 32.3000], // Mityana direction
];

export default function MapInput() {
  const [points, setPoints] = useState(POINTS);

  function addPoint(point) {
    setPoints((prev) => [...prev, point]);
  }

  return (
    <div>
      <MapContainer
        center={[0.3476, 32.5825]} // Kampala
        zoom={10}
        style={{ height: "500px", width: "100%" }}
      >
        <TileLayer
          attribution="&copy; OpenStreetMap contributors"
          url="https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png"
        />

        <MapClickHandler addPoint={addPoint} />

        {points.map((point, index) => (
          <Marker key={index} position={point}>
            <Popup>
              <strong>Point {index + 1}</strong><br />
              Lat: point.[0]<br />
              Lng: point.[1]
            </Popup>
          </Marker>
        ))}
      </MapContainer>
    </div>
  );
}