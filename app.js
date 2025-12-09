// API Configuration - AWS Backend
const API_BASE = 'http://13.60.243.32:8080/api';

// Global state
let junctions = [];

// Initialize on page load
document.addEventListener('DOMContentLoaded', () => {
    checkServerStatus();
    loadJunctions();
    setupEventListeners();
});

// Check server status
async function checkServerStatus() {
    const statusEl = document.getElementById('serverStatus');
    const statusText = document.getElementById('statusText');
    
    try {
        const response = await fetch(`${API_BASE}/health`);
        const data = await response.json();
        
        if (data.status === 'OK') {
            statusEl.classList.add('online');
            statusText.textContent = `AWS Server Online • ${data.junctions} Junctions`;
        }
    } catch (error) {
        statusEl.classList.add('offline');
        statusText.textContent = 'Server Offline';
        showToast('Cannot connect to AWS server!', 'error');
    }
}

// Load all junctions
async function loadJunctions() {
    try {
        const response = await fetch(`${API_BASE}/junctions`);
        const data = await response.json();
        junctions = data.junctions;
        
        populateDropdowns();
        displayJunctionsList();
        
        showToast(`Loaded ${junctions.length} junctions from AWS!`, 'success');
    } catch (error) {
        console.error('Error loading junctions:', error);
        showToast('Failed to load junctions', 'error');
    }
}

// Populate dropdown menus
function populateDropdowns() {
    const sourceSelect = document.getElementById('sourceSelect');
    const destSelect = document.getElementById('destSelect');
    const trafficFrom = document.getElementById('trafficFrom');
    const trafficTo = document.getElementById('trafficTo');
    
    [sourceSelect, destSelect, trafficFrom, trafficTo].forEach(select => {
        select.innerHTML = '<option value="">Select...</option>';
    });
    
    junctions.forEach(junction => {
        const option = `<option value="${junction.id}">${junction.name}</option>`;
        sourceSelect.innerHTML += option;
        destSelect.innerHTML += option;
        trafficFrom.innerHTML += option;
        trafficTo.innerHTML += option;
    });
}

// Display junctions list
function displayJunctionsList() {
    const junctionsList = document.getElementById('junctionsList');
    
    junctionsList.innerHTML = junctions.map(j => `
        <div class="junction-item">
            <div class="junction-name">${j.name}</div>
            <div class="junction-id">ID: ${j.id}</div>
            <div class="junction-coords">${j.lat}°N, ${j.lng}°E</div>
        </div>
    `).join('');
}

// Setup event listeners
function setupEventListeners() {
    document.getElementById('findRouteBtn').addEventListener('click', findRoute);
    document.getElementById('updateTrafficBtn').addEventListener('click', updateTraffic);
}

// Find shortest route
async function findRoute() {
    const sourceId = document.getElementById('sourceSelect').value;
    const destId = document.getElementById('destSelect').value;
    
    if (!sourceId || !destId) {
        showToast('Please select both source and destination', 'error');
        return;
    }
    
    if (sourceId === destId) {
        showToast('Source and destination cannot be the same!', 'error');
        return;
    }
    
    const btn = document.getElementById('findRouteBtn');
    btn.disabled = true;
    btn.textContent = 'Finding Route...';
    
    try {
        const response = await fetch(`${API_BASE}/path`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({
                source: parseInt(sourceId),
                destination: parseInt(destId)
            })
        });
        
        const data = await response.json();
        
        if (data.success) {
            displayRoute(data);
            showToast('Route found successfully!', 'success');
        } else {
            showToast(data.message || 'No route found', 'error');
        }
    } catch (error) {
        console.error('Error finding route:', error);
        showToast('Failed to find route', 'error');
    } finally {
        btn.disabled = false;
        btn.textContent = 'Find Shortest Route';
    }
}

// Display route results
function displayRoute(data) {
    const resultsCard = document.getElementById('resultsCard');
    const totalTime = document.getElementById('totalTime');
    const totalDistance = document.getElementById('totalDistance');
    const pathDisplay = document.getElementById('pathDisplay');
    
    resultsCard.style.display = 'block';
    resultsCard.scrollIntoView({ behavior: 'smooth', block: 'nearest' });
    
    totalTime.textContent = `${data.totalTime.toFixed(1)} min`;
    totalDistance.textContent = `${data.estimatedDistance.toFixed(1)} km`;
    
    pathDisplay.innerHTML = data.path.map((junction, index) => {
        const stepHtml = `<div class="path-step">${junction.name}</div>`;
        const arrowHtml = index < data.path.length - 1 ? '<div class="path-arrow">→</div>' : '';
        return stepHtml + arrowHtml;
    }).join('');
}

// Update traffic conditions
async function updateTraffic() {
    const fromId = document.getElementById('trafficFrom').value;
    const toId = document.getElementById('trafficTo').value;
    const multiplier = parseFloat(document.getElementById('trafficLevel').value);
    
    if (!fromId || !toId) {
        showToast('Please select both junctions', 'error');
        return;
    }
    
    if (fromId === toId) {
        showToast('From and To junctions cannot be the same!', 'error');
        return;
    }
    
    const btn = document.getElementById('updateTrafficBtn');
    btn.disabled = true;
    btn.textContent = 'Updating...';
    
    try {
        const response = await fetch(`${API_BASE}/traffic`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({
                from: parseInt(fromId),
                to: parseInt(toId),
                multiplier: multiplier
            })
        });
        
        const data = await response.json();
        
        if (data.success) {
            const fromName = junctions.find(j => j.id == fromId)?.name || fromId;
            const toName = junctions.find(j => j.id == toId)?.name || toId;
            showToast(`Traffic updated: ${fromName} ↔ ${toName} (${multiplier}x)`, 'success');
            
            document.getElementById('trafficFrom').value = '';
            document.getElementById('trafficTo').value = '';
            document.getElementById('trafficLevel').value = '1.0';
        } else {
            showToast(data.message || 'Failed to update traffic', 'error');
        }
    } catch (error) {
        console.error('Error updating traffic:', error);
        showToast('Failed to update traffic', 'error');
    } finally {
        btn.disabled = false;
        btn.textContent = 'Update Traffic';
    }
}

// Show toast notification
function showToast(message, type = 'success') {
    const toast = document.getElementById('toast');
    toast.textContent = message;
    toast.className = `toast ${type} show`;
    
    setTimeout(() => {
        toast.classList.remove('show');
    }, 3000);
}

// Auto-refresh server status
setInterval(checkServerStatus, 30000);