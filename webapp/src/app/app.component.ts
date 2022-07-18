import { Component } from '@angular/core';
import { Title } from '@angular/platform-browser';
import { MatDialog } from '@angular/material/dialog';
import { Subscription } from 'rxjs';
import { ModelService } from './model.service';
import { ConnectingDialogComponent } from './connecting-dialog/connecting-dialog.component';
import { SettingsDialogComponent } from './settings-dialog/settings-dialog.component';
import { RebootDialogComponent } from './reboot-dialog/reboot-dialog.component';
import { CalibrateDialogComponent } from './calibrate-dialog/calibrate-dialog.component';

@Component({
  selector: 'app-root',
  templateUrl: './app.component.html',
  styleUrls: ['./app.component.css']
})
export class AppComponent {

  title: string = '';

  private modeSubscription!: Subscription;
  private nameSubscription!: Subscription;
  private connectedSubscription!: Subscription;

  constructor(
    private titleService: Title,
    public model: ModelService,
    private dialog: MatDialog
  ) {}

  ngOnInit(): void {
    this.modeSubscription = this.model.mode.asObservable().subscribe(s => {this.setTitle()});
    this.nameSubscription = this.model.name.asObservable().subscribe(s => {this.setTitle()});
    this.connectedSubscription = this.model.name.asObservable().subscribe(b => {this.showConnecting(!b)});
  }

  ngOnDestroy() {
    this.modeSubscription.unsubscribe();
    this.nameSubscription.unsubscribe();
    this.connectedSubscription.unsubscribe();
  }

  setTitle(): void {
    this.title = this.model.mode.value;
    if (this.model.name.value) this.title += ': ' + this.model.name.value;
    this.titleService.setTitle(this.title);
  }

  showConnecting(show: boolean): void {
    this.dialog.open(ConnectingDialogComponent);
  }

  settings(): void {
    this.dialog.open(SettingsDialogComponent);
  }

  calibrate(): void {
    this.dialog.open(CalibrateDialogComponent);
  }
  
  saveConfig(): void {
    this.model.saveConfig().subscribe({
      next: (res: string) => console.info('saveConfig: ' + res)
    });
  }

  reboot(): void {
    this.dialog.open(RebootDialogComponent);
  }

}
