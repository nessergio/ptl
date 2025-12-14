# Deployment Guide

Warehouse and industrial deployment guide for the Pick-to-Light system.

## Table of Contents

- [Deployment Scenarios](#deployment-scenarios)
- [Pre-Deployment Planning](#pre-deployment-planning)
- [Installation Process](#installation-process)
- [Configuration Strategies](#configuration-strategies)
- [Training and Onboarding](#training-and-onboarding)
- [Performance Optimization](#performance-optimization)
- [Monitoring and Maintenance](#monitoring-and-maintenance)

---

## Deployment Scenarios

### Scenario 1: Order Picking (Pick-to-Light)

**Use Case:** Workers collect items for customer orders based on Bill of Materials (BOM).

**Workflow:**
1. Worker scans order barcode or item barcode
2. LED illuminates at the shelf containing the item
3. Worker retrieves item from highlighted shelf
4. Process repeats for each item in the order
5. Order completed when all items collected

**Configuration:**
- Map item barcodes to shelf locations
- One LED per shelf/bin location
- Consider color coding by priority (requires RGB LEDs)

**Benefits:**
- Reduces pick errors
- Faster picking times
- Less training required for new workers
- Real-time pick confirmation

---

### Scenario 2: Package Sorting

**Use Case:** Sort incoming packages to destination bins based on delivery routes or zip codes.

**Workflow:**
1. Worker scans package barcode or shipping label
2. LED illuminates at destination bin/shelf
3. Worker places package in highlighted location
4. System logs sort event
5. Process repeats for next package

**Configuration:**
- Map tracking numbers or zip codes to sort destinations
- Group destinations logically (routes, regions, carriers)
- Use multiple devices for high-volume operations

**Benefits:**
- Eliminates manual sorting errors
- Faster throughput
- Flexible routing changes
- Audit trail of sorted packages

---

### Scenario 3: Inventory Management

**Use Case:** Guide workers to storage locations for putaway or cycle counting.

**Workflow:**
1. Scan item to be stored
2. System directs to optimal storage location
3. LED highlights the shelf
4. Worker places item and confirms
5. Inventory system updated

**Configuration:**
- Dynamic mapping based on inventory system
- Integration with WMS (Warehouse Management System)
- Consider API integration for real-time updates

---

### Scenario 4: Assembly Operations

**Use Case:** Guide assembly workers to component locations based on production schedule.

**Workflow:**
1. Scan assembly work order
2. LEDs highlight components needed in sequence
3. Worker collects components in order
4. Assembly proceeds efficiently
5. Quality checks reduced

**Configuration:**
- Map work orders to component locations
- Sequential highlighting possible with custom code
- Multiple devices for complex assemblies

---

## Pre-Deployment Planning

### Step 1: Site Survey

**Physical Assessment:**
- [ ] Measure shelf dimensions and spacing
- [ ] Identify power outlet locations
- [ ] Check WiFi coverage in warehouse area
- [ ] Note any environmental factors (temperature, dust, moisture)
- [ ] Document lighting conditions (LED visibility)
- [ ] Measure cable run distances

**Network Assessment:**
- [ ] Test WiFi signal strength at device location
- [ ] Identify available network ports (if using wired)
- [ ] Check network security requirements
- [ ] Verify firewall rules for device access
- [ ] Plan IP addressing (static vs DHCP)

**Operational Assessment:**
- [ ] Map current workflow
- [ ] Identify integration points
- [ ] Determine barcode standards in use
- [ ] Count number of pick/sort locations
- [ ] Estimate daily transaction volume
- [ ] Identify peak operation hours

### Step 2: System Design

**Hardware Sizing:**
```
Number of LED indicators needed = Number of pick/sort locations

Number of CH423 chips needed = ⌈LED count / 24⌉

Number of ESP32 devices needed = ⌈LED count / 48⌉
```

**Example:**
- 80 shelf locations → 2 ESP32 devices (48 LEDs each)
- OR: 1 ESP32 + custom expansion (requires code modification)

**Network Design:**
- Decide: WiFi vs Ethernet
- Plan: Access points if WiFi
- Document: IP addresses for each device
- Consider: VLANs for device isolation

**Power Design:**
- Calculate total current draw per device
- Plan power distribution to shelves
- Consider: PoE for Ethernet installations
- Budget: Battery backup for critical operations

### Step 3: Barcode Mapping Strategy

**Mapping Approaches:**

1. **Static Mapping** (suitable for stable inventory):
   - Create `table.json` with all products
   - Update periodically as inventory changes
   - Simple, no external dependencies

2. **Dynamic Mapping** (suitable for changing inventory):
   - Use API to update mappings in real-time
   - Integrate with WMS/ERP system
   - Requires custom middleware

3. **Hybrid Mapping**:
   - Static mapping for permanent items
   - Dynamic updates for seasonal/temporary items
   - Balance between flexibility and simplicity

**Naming Conventions:**

Choose logical, scalable names for locations:

```
Good Examples:
- "A-01-01" (Aisle-Bay-Level)
- "RECV-BIN-5" (Receiving Bin 5)
- "ROUTE-42-A" (Delivery Route 42, Bin A)

Bad Examples:
- "ShelfA" (doesn't scale)
- "LED5" (not descriptive)
- "Location1" (ambiguous)
```

### Step 4: Procurement

**Shopping List Template:**

```
Quantity | Item | Notes
---------|------|------
____     | ESP32-DOIT-DevKit-V1 | 1 per 48 LEDs
____     | CH423 I/O Expander | 1-2 per ESP32
____     | High-brightness LED indicators | Red, green, or RGB
____     | 5V Power Supplies | 10A minimum per device
____     | Ethernet cable (optional) | If not using WiFi
____     | Mounting enclosures | IP rating for environment
____     | Wire (22-18 AWG) | For LED connections
____     | Resistors (220Ω) | Current limiting for LEDs
____     | BLE Barcode Scanners | 1 per picking station
____     | Labels/Signage | For shelf identification
```

---

## Installation Process

### Phase 1: Pilot Installation (Week 1)

**Goals:**
- Test system with 5-10 locations
- Validate workflow
- Train core team
- Identify issues

**Steps:**
1. Install one device with 5-10 LEDs
2. Configure basic barcode mappings
3. Pair one scanner
4. Test with actual workflow
5. Gather feedback from workers

**Success Criteria:**
- [ ] All LEDs functional
- [ ] Scanner pairs reliably
- [ ] Barcodes trigger correct LEDs
- [ ] Workers can use system with minimal training
- [ ] No network connectivity issues

### Phase 2: Section Rollout (Week 2-3)

**Goals:**
- Expand to full aisle or section
- Install all LEDs for pilot area
- Run parallel with existing process

**Steps:**
1. Install remaining devices for section
2. Mount all LEDs on shelves
3. Create complete barcode mapping
4. Train all workers in section
5. Run dual system (old + new) for validation

**Success Criteria:**
- [ ] 100% of locations covered
- [ ] Pick accuracy improved vs old method
- [ ] Worker feedback positive
- [ ] System uptime >99%

### Phase 3: Full Deployment (Week 4+)

**Goals:**
- Roll out to entire facility
- Decommission old system
- Establish maintenance procedures

**Steps:**
1. Install devices in all areas
2. Complete all barcode mappings
3. Train all staff
4. Monitor performance metrics
5. Document procedures

---

## Configuration Strategies

### Small Warehouse (< 50 locations)

**Recommended Setup:**
- 1 ESP32 device
- 1-2 CH423 chips
- Single WiFi access point
- Manual config file management

**Configuration:**
```json
{
    "standalone": false,
    "ssid": "WarehouseWiFi",
    "pins": ["A1", "A2", "A3", ..., "A48"]
}
```

**Maintenance:**
- Update `table.json` monthly or as needed
- Manual firmware updates
- Simple backup strategy (copy config files)

### Medium Warehouse (50-200 locations)

**Recommended Setup:**
- 2-4 ESP32 devices
- Dedicated WiFi network
- Network file share for configs
- Centralized monitoring dashboard

**Configuration:**
```json
{
    "device_id": "PTL-ZONE-A",
    "standalone": false,
    "ssid": "WarehouseWiFi-PTL",
    "pins": ["ZA-01", "ZA-02", ..., "ZA-48"]
}
```

**Automation:**
- Script to deploy configs to multiple devices
- Scheduled barcode mapping updates
- Centralized logging
- Automated health checks

### Large Warehouse (200+ locations)

**Recommended Setup:**
- 5+ ESP32 devices
- Managed WiFi with multiple APs
- API integration with WMS
- Real-time monitoring system
- Redundancy for critical areas

**Architecture:**
```
┌─────────────┐
│ WMS/ERP     │
│ System      │
└──────┬──────┘
       │ API
       ▼
┌─────────────┐      ┌─────────────┐
│ Middleware  │◄────►│ Monitoring  │
│ Server      │      │ Dashboard   │
└──────┬──────┘      └─────────────┘
       │ HTTP/MQTT
       ▼
┌─────────────┬─────────────┬─────────────┐
│ PTL Device  │ PTL Device  │ PTL Device  │
│ Zone A      │ Zone B      │ Zone C      │
│ (48 LEDs)   │ (48 LEDs)   │ (48 LEDs)   │
└─────────────┴─────────────┴─────────────┘
```

**Integration Example:**

Python middleware for WMS integration:
```python
import requests
import mysql.connector

# WMS Database connection
wms_db = mysql.connector.connect(
    host="wms.example.com",
    user="ptl_user",
    password="password",
    database="warehouse"
)

# PTL device endpoints
devices = {
    "ZoneA": "http://192.168.1.101",
    "ZoneB": "http://192.168.1.102",
    "ZoneC": "http://192.168.1.103"
}

def update_barcode_mappings():
    """Update PTL mappings from WMS database"""
    cursor = wms_db.cursor()
    cursor.execute("""
        SELECT item_barcode, shelf_location, zone
        FROM inventory
        WHERE active = 1
    """)

    mappings = {}
    for barcode, location, zone in cursor:
        if zone not in mappings:
            mappings[zone] = {}
        if location not in mappings[zone]:
            mappings[zone][location] = []
        mappings[zone][location].append(barcode)

    # Upload to each device
    for zone, device_url in devices.items():
        if zone in mappings:
            upload_table(device_url, mappings[zone])

def upload_table(device_url, table_data):
    """Upload table.json to device"""
    # Implementation depends on how you expose config updates
    # Could use modified /api/writeConfig or custom endpoint
    pass
```

---

## Training and Onboarding

### Worker Training Program

**Session 1: System Overview (15 minutes)**
- What is pick-to-light and why we're using it
- Benefits for workers (easier, faster, fewer errors)
- Basic system components

**Session 2: Hands-On Practice (30 minutes)**
1. How to use the barcode scanner
2. Scanning items and watching LEDs
3. What to do if LED doesn't light up
4. How to confirm pick completion
5. Practice with sample orders

**Session 3: Troubleshooting (15 minutes)**
- Scanner not working → check battery, re-pair
- LED not lighting → report to supervisor
- Wrong LED lights up → verify barcode, check mapping
- System slow → check WiFi indicator

**Training Materials:**
- Quick reference card (laminated)
- Video demonstration
- FAQ sheet
- Contact info for tech support

### IT Staff Training

**Topics:**
- Device network configuration
- Firmware update procedure
- Configuration file management
- Barcode mapping updates
- Log file analysis
- Common hardware issues
- Backup and restore procedures

---

## Performance Optimization

### Network Optimization

**WiFi Best Practices:**
- Use 2.4GHz for better range
- Dedicated SSID for PTL devices
- Minimum signal strength: -70 dBm
- Enable QoS for PTL traffic
- Limit devices per AP (< 20 devices)

**Reducing Latency:**
- Place ESP32 devices close to AP
- Minimize WiFi interference
- Use wired Ethernet for high-throughput areas
- Disable unnecessary services on ESP32

### System Tuning

**Adjust blink parameters** (in `src/blink.cpp`):

```cpp
// Fast pick operations
unsigned long blinkDuration = 5000UL;   // 5 seconds
int blinkPeriod = 500;                  // Fast blink
int blinkFill = 250;

// Slow pick operations (heavy items)
unsigned long blinkDuration = 30000UL;  // 30 seconds
int blinkPeriod = 1000;                 // Slow blink
int blinkFill = 500;

// Very visible (noisy environment)
int blinkPeriod = 100;                  // Very fast
int blinkFill = 50;
```

**Adjust scan timing** (in `src/ble.cpp`):
```cpp
// More frequent scanner checks
unsigned long timerDelay = 5000;  // Check every 5 seconds

// Less frequent (save power)
unsigned long timerDelay = 30000; // Check every 30 seconds
```

### LED Brightness

For better visibility in bright warehouses:
- Use high-brightness LEDs (>1000 mcd)
- Add diffusers to LEDs
- Use RGB LEDs with custom colors per priority
- Angle LEDs toward worker sight lines

---

## Monitoring and Maintenance

### Daily Monitoring

**Automated Checks:**
```bash
#!/bin/bash
# Daily health check script

DEVICES=("192.168.1.101" "192.168.1.102" "192.168.1.103")

for ip in "${DEVICES[@]}"; do
    echo "Checking device at $ip..."

    # Check if device responds
    if ! curl -s --max-time 5 "http://$ip/api/wifiInfo" > /dev/null; then
        echo "ALERT: Device $ip not responding!"
        # Send notification
    fi

    # Check scanner connection (parse SSE events)
    timeout 15 curl -s "http://$ip/events" | grep -q "\"status\":2"
    if [ $? -ne 0 ]; then
        echo "WARNING: Scanner not connected on $ip"
    fi
done
```

### Weekly Maintenance

**Checklist:**
- [ ] Test all LEDs (use pin 48 test)
- [ ] Check scanner battery levels
- [ ] Review error logs
- [ ] Verify WiFi signal strength
- [ ] Update barcode mappings
- [ ] Test backup scanners
- [ ] Clean LED lenses
- [ ] Check cable connections

### Monthly Maintenance

**Tasks:**
- Firmware update check
- Replace any failed LEDs
- Review performance metrics
- Update documentation
- Refresh worker training
- Test disaster recovery procedure

### Performance Metrics

**Track These KPIs:**
```
Metric                    Target
---------------------------------
System uptime             >99%
Pick accuracy             >99.5%
Average pick time         <30 seconds
Scanner connection rate   >95%
LED failure rate          <1%
False scans               <0.5%
```

### Backup and Disaster Recovery

**Backup Strategy:**

```bash
#!/bin/bash
# Backup all device configs

BACKUP_DIR="/backups/ptl/$(date +%Y%m%d)"
mkdir -p "$BACKUP_DIR"

for ip in "${DEVICES[@]}"; do
    # Backup config (requires custom endpoint or manual SPIFFS extraction)
    device_name=$(echo $ip | tr '.' '-')
    # Copy config files
    cp "config_${device_name}.json" "$BACKUP_DIR/"
    cp "table_${device_name}.json" "$BACKUP_DIR/"
done

# Keep 30 days of backups
find /backups/ptl -type d -mtime +30 -exec rm -rf {} \;
```

**Recovery Procedure:**
1. Replace failed ESP32 device
2. Flash firmware from PlatformIO
3. Upload backed-up config files
4. Upload backed-up table.json
5. Test all LEDs
6. Pair scanner
7. Verify operation

---

## Troubleshooting Common Issues

### High Error Rate

**Symptoms:** Workers reporting wrong LEDs lighting up

**Causes:**
- Incorrect barcode mapping
- Similar barcodes causing confusion
- LED labels don't match mapping

**Solutions:**
- Audit `table.json` against actual inventory
- Add check digits to custom barcodes
- Re-label LEDs with correct location names
- Train workers on barcode scanning technique

### Slow Response Time

**Symptoms:** Delay between scan and LED illumination

**Causes:**
- Poor WiFi signal
- Network congestion
- Multiple devices sharing AP
- BLE interference

**Solutions:**
- Move AP closer or add APs
- Use 5GHz WiFi where possible (requires ESP32 with 5GHz support)
- Reduce BLE scan interval
- Check for other BLE devices causing interference

### Scanner Connection Drops

**Symptoms:** Scanner disconnects frequently

**Causes:**
- Low battery
- BLE interference
- Out of range
- Scanner compatibility

**Solutions:**
- Replace scanner batteries
- Move closer to ESP32
- Update BLE scanner firmware
- Try different scanner model
- Reduce WiFi transmit power to minimize interference

---

## Scaling Considerations

### Growing from Pilot to Enterprise

**10-50 devices:**
- Standardize naming conventions
- Implement centralized config management
- Deploy monitoring dashboard
- Document runbooks

**50+ devices:**
- Consider custom firmware for your needs
- Implement automated deployment pipeline
- Add redundancy and failover
- Integrate with enterprise systems (WMS, ERP)
- Dedicated IT staff for PTL system

### Integration with WMS

**Integration Points:**
- Real-time barcode mapping updates
- Pick confirmation back to WMS
- Performance metrics reporting
- Inventory location synchronization

**Technologies:**
- REST API (current system)
- MQTT (requires firmware modification)
- Database integration via middleware
- File sync (simple but less real-time)

---

## Cost Analysis

### Per-Location Cost Estimate

```
Component                Cost (USD)    Notes
---------------------------------------------------
ESP32 board              $8            Per 48 locations
CH423 chip               $3            Per 24 locations
LED indicator            $2-5          Per location
Wiring & connectors      $1            Per location
Power supply             $20           Per device
Enclosure                $15           Per device
Installation labor       $5            Per location
---------------------------------------------------
Total per location:      ~$10-15       (amortized)
```

### ROI Calculation

**Cost Savings:**
- Reduced pick errors: $X per error avoided
- Faster picking: Y seconds saved per pick
- Less training time: Z hours saved per worker
- Lower turnover: Easier job = happier workers

**Example:**
```
Warehouse with 100 picks/hour:
- Error rate reduction: 5% → 1% = 4 errors/hour saved
- Time savings: 10 seconds/pick = 16.7 minutes/hour
- Cost of error: $50 (reprocessing, shipping)
- Labor cost: $20/hour

Savings per hour: 4 × $50 + (16.7/60) × $20 = $205.67
Savings per year (2000 hours): $411,340
System cost (100 locations): $1,500
ROI: 27,422% or payback in 2 days
```

---

## Future Enhancements

**Planned features:**
- Voice guidance integration
- Multi-color LED priorities
- Automatic inventory updates
- Predictive maintenance
- Mobile app for remote management
- Cloud dashboard for multi-site

---

## Support and Resources

**Internal Resources:**
- System administrator: [contact]
- Vendor support: [contact]
- Training materials: [location]
- Spare parts: [location]

**External Resources:**
- ESP32 documentation
- PlatformIO forums
- This project's GitHub/repository

---

**Document Version:** 1.0
**Last Updated:** [Date]
**Prepared by:** [Your team]
