// ===================================================================
// AI SMART GLASSES - ALL-IN-ONE TEMPLE CLIP ENCLOSURE (3D PRINTABLE)
// Project: AI-Powered Smart Glasses using ESP32-CAM
// Units: Millimeters (mm)
// ===================================================================

$fn = 60; // Smooth curves

// Enclosure Dimensions
box_l = 54;  // Length along temple arm (X axis)
box_w = 32;  // Width/depth (Y axis)
box_h = 18;  // Height (Z axis)
wall  = 2;   // Wall thickness

// Component Dimensions & Offsets
cam_aperture_d = 9;   // Camera lens diameter
mic_aperture_d = 3;   // Microphone hole diameter
oled_win_l     = 26;  // OLED screen length
oled_win_w     = 14;  // OLED screen width
usb_slot_w     = 10;  // USB-C/Micro-USB cutout width
usb_slot_h     = 6;   // USB-C cutout height

// Clip Dimensions (Fits glasses stem 4mm - 8mm height, 2.5mm - 4mm thickness)
clip_gap_w     = 4.5; // Gap for glasses arm
clip_wall      = 2.5; // Wall thickness of clip C-channel

module main_enclosure() {
    difference() {
        // Outer Shell
        union() {
            // Main housing box
            cube([box_l, box_w, box_h]);
            
            // Integrated Glasses C-Clip on the Inner Back Face (Y = box_w)
            translate([4, box_w, 2])
                difference() {
                    cube([box_l - 8, clip_gap_w + clip_wall * 2, box_h - 4]);
                    // Inner channel cutout for glasses arm
                    translate([-1, clip_wall, clip_wall])
                        cube([box_l, clip_gap_w, box_h]);
                    // Angled entry chamfer for easy snap-on
                    translate([-1, clip_wall, box_h - 6])
                        rotate([15, 0, 0])
                            cube([box_l, clip_gap_w + 2, 10]);
                }
        }
        
        // Inner Cavity (Hollow Inside)
        translate([wall, wall, wall])
            cube([box_l - wall*2, box_w - wall*2, box_h - wall*2]);
            
        // -------------------------------------------------------------
        // CUTOUTS & APERTURES
        // -------------------------------------------------------------
        
        // 1. Camera Aperture (Front Face, X = 0)
        translate([-1, 16, box_h / 2])
            rotate([0, 90, 0])
                cylinder(d = cam_aperture_d, h = wall + 2);
                
        // 2. Microphone Port (Front Face, X = 0)
        translate([-1, 5, 5])
            rotate([0, 90, 0])
                cylinder(d = mic_aperture_d, h = wall + 2);

        // 3. OLED Screen Viewport (Top Face, Z = box_h)
        translate([12, 8, box_h - wall - 1])
            cube([oled_win_l, oled_win_w, wall + 3]);
            
        // 4. USB-C Charging Port Slot (Rear Face, X = box_l)
        translate([box_l - wall - 1, (box_w - usb_slot_w)/2, 3])
            cube([wall + 3, usb_slot_w, usb_slot_h]);
            
        // 5. Speaker Sound Grille (Bottom Face, Z = 0)
        for (i = [0:3]) {
            translate([16 + (i * 4), 20, -1])
                cube([2, 8, wall + 2]);
        }
        
        // 6. Power Switch Cutout (Rear Side, X = box_l)
        translate([box_l - wall - 1, 4, box_h - 8])
            cube([wall + 3, 7, 4]);
    }
    
    // Internal PCB Standoff Mounting Pegs
    translate([wall + 3, wall + 3, wall])
        cylinder(d=3, h=3);
    translate([wall + 3, box_w - wall - 3, wall])
        cylinder(d=3, h=3);
    translate([box_l - wall - 10, wall + 3, wall])
        cylinder(d=3, h=3);
    translate([box_l - wall - 10, box_w - wall - 3, wall])
        cylinder(d=3, h=3);
}

// Render the 3D Enclosure
main_enclosure();
