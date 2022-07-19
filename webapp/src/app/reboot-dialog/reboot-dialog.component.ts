import { Component } from '@angular/core';
import { MatDialogRef } from '@angular/material/dialog';
import { ModelService } from '../model.service';

@Component({
  selector: 'app-reboot-dialog',
  templateUrl: './reboot-dialog.component.html',
  styleUrls: ['./reboot-dialog.component.css']
})
export class RebootDialogComponent {

  constructor(
    private dialogRef: MatDialogRef<RebootDialogComponent>,
    public model: ModelService) {
      dialogRef.disableClose = true;
  }

  reboot(): void {
    this.model.reboot().subscribe({
      next: (res: any) => {
        console.info('reboot: ' + res);
        if (res)
          this.dialogRef.close(true);
      }
    });
  }

  cancel(): void {
    this.dialogRef.close(false);
  }

}
