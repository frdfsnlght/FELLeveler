import { Component, OnInit, OnDestroy } from '@angular/core';
import { MatDialogRef } from '@angular/material/dialog';
import { ModelService } from '../model.service';

@Component({
  selector: 'app-unpair-dialog',
  templateUrl: './unpair-dialog.component.html',
  styleUrls: ['./unpair-dialog.component.css']
})
export class UnpairDialogComponent implements OnInit, OnDestroy {

  mode: string = '';
  
  constructor(
    private dialogRef: MatDialogRef<UnpairDialogComponent>,
    public model: ModelService) {
      this.mode = model.mode.value;
  }

  ngOnInit(): void {}

  ngOnDestroy() {}

  removePairing(address: string): void {
    this.model.unpairBTDevice(address).subscribe({
      next: (res: string) => {
        console.info('unpairBTDevice: ' + res);
        if (res == 'OK')
          this.dialogRef.close(true);
      }
    });
  }

  cancel(): void {
    this.dialogRef.close(false);
  }

}
