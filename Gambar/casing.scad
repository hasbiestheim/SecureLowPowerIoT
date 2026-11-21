// 1-inch Pipe + Internal "PCB" (OpenSCAD)
// Units: mm (1 inch = 25.4 mm)
$fn = 128;

// ===== Parameters =====
inner_d_inch   = 1;           // nominal inner diameter (inches)
inner_d        = inner_d_inch * 25.4;
pipe_len       = 120;         // pipe length (mm)
wall           = 2.0;         // pipe wall thickness
chamfer        = 0.8;         // small outer bevel at the top

// Cap (slip-on) params
cap_wall       = 3.0;
cap_top_thick  = 3.0;
cap_depth      = 15.0;
clearance      = 0.25;        // slip fit for cap over pipe OD

// Internal "PCB" (green) params
pcb_gap        = 1.0;         // clearance from inner wall
pcb_thickness  = 1.0;         // typical PCB thickness
pcb_width      = inner_d - 2*pcb_gap;
pcb_height     = pipe_len;    // reach top

// ===== Derived =====
outer_d        = inner_d + 2*wall;
cap_inner_d    = outer_d + 2*clearance;
cap_outer_d    = cap_inner_d + 2*cap_wall;

// ===== Helpers =====
module ring_cyl(od, id, h) {
    difference() { cylinder(d=od, h=h); cylinder(d=id, h=h+0.2); }
}

// ===== Parts =====
// Pipe body (open bore end-to-end)
module pipe() {
    color([0.8, 0.9, 1.0])  // light PVC bluish-gray
    difference() {
        cylinder(d=outer_d, h=pipe_len);
        translate([0,0,-0.1]) cylinder(d=inner_d, h=pipe_len+0.2);
    }
    // small external chamfer at the top rim
   // translate([0,0,pipe_len - chamfer])
     //   ring_cyl(outer_d, outer_d - 2*chamfer, chamfer);
}

// Internal vertical PCB board
module pcb_board() {
    color([0.0, 4.5, 0.4])  // bright green
    translate([-pcb_thickness/2, -pcb_width/2, 10])  // anchor at bottom
            cube([pcb_thickness, pcb_width, pcb_height], center=false);
}

// Cap (slip-on)
module cap() {
    union() {
        color([0.8, 0.9, 1.0])  // PVC same as pipe
        ring_cyl(cap_outer_d, cap_inner_d, cap_depth);
        translate([0,0,cap_depth]) color([0.8, 0.9, 1.0])
        cylinder(d=cap_outer_d, h=cap_top_thick);
        // small entry bevel
        color([0.8, 0.9, 1.0])  // PVC same as pipe
        ring_cyl(cap_outer_d, cap_outer_d - 2*chamfer, chamfer);
    }
}

// ===== Arrange on build plate =====
// Pipe + internal PCB
union() {
    pipe();
    pcb_board();
}

// Place cap to the side
translate([outer_d*1.3, 0, 0]) cap();
